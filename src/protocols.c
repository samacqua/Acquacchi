// protocols.c

#include "stdio.h"
#include "string.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

#define INPUTBUFFER 400 * 6

// 3 Protocols to play chess.
//  1. Universal Chess Interface
//  2. XBoard
//  3. Console

// MARK: Universal Chess Interface

// example command: go depth 5 wtime 10000 btime 10000 binc 3000 winc 3000 movetime 1000 movestogo 20
void ParseGo(char* line, Search *info, BoardState *pos) {
    
    // if depth and movetime are not set, then we will know bc still -1. If movestogo not set, divide time by 30.
    int depth = -1, movestogo = 30,movetime = -1;
    int time = -1, inc = 0;
    char *ptr = NULL;
    info->timeset = FALSE;
    
    if ((ptr = strstr(line,"infinite"))) {
        ;
    }
    
    // for each command, go to beginning of actual data (add length of command)
    if ((ptr = strstr(line,"binc")) && pos->side == BLACK) {
        inc = atoi(ptr + 5);
    }
    
    if ((ptr = strstr(line,"winc")) && pos->side == WHITE) {
        inc = atoi(ptr + 5);
    }
    
    if ((ptr = strstr(line,"wtime")) && pos->side == WHITE) {
        time = atoi(ptr + 6);
    }
    
    if ((ptr = strstr(line,"btime")) && pos->side == BLACK) {
        time = atoi(ptr + 6);
    }
    
    if ((ptr = strstr(line,"movestogo"))) {
        movestogo = atoi(ptr + 10);
    }
    
    if ((ptr = strstr(line,"movetime"))) {
        movetime = atoi(ptr + 9);
    }
    
    if ((ptr = strstr(line,"depth"))) {
        depth = atoi(ptr + 6);
    }
    
    // command specified movetime, so tell engine to spend that time on next analysis
    if(movetime != -1) {
        time = movetime;
        movestogo = 1;
    }
    
    info->starttime = GetTimeMs();
    info->depth = depth;
    
    // command specified time
    if(time != -1) {
        info->timeset = TRUE;
        time /= movestogo;
        time -= 50;
        info->stoptime = info->starttime + time + inc;
    }
    
    // depth unspecified, set to max depth
    if(depth == -1) {
        info->depth = MAXSEARCHDEPTH;
    }
    
    // show engine thinking
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
           time,info->starttime,info->stoptime,info->depth,info->timeset);
    SearchPosition(pos, info);
}

/*  3 formats to parse:
 1. position fen fenstr
 2. position startpos
 3. ...moves d2d4 d7d5 c1f4
 */
void ParsePosition(char* lineIn, BoardState *pos) {
    
    // move pointer to commandline forward 9 characters to start at FEN or start pos
    lineIn += 9;
    char *ptrChar = lineIn;
    
    // set position to start or FEN
    if(strncmp(lineIn, "startpos", 8) == 0){
        ParseFEN(START_FEN, pos);
    } else {
        ptrChar = strstr(lineIn, "fen");
        if(ptrChar == NULL) {
            ParseFEN(START_FEN, pos);
        } else {
            ptrChar+=4;
            ParseFEN(ptrChar, pos);
        }
    }
    
    ptrChar = strstr(lineIn, "moves");
    int move;
    
    // make move
    if(ptrChar != NULL) {
        ptrChar += 6;
        while(*ptrChar) {
            move = ParseMove(ptrChar,pos);
            if(move == NOMOVE) break;
            MakeMove(pos, move);
            pos->ply=0;
            
            // go to next move
            while(*ptrChar && *ptrChar!= ' ') ptrChar++;
            ptrChar++;
        }
    }
    PrintBoard(pos);
}

void Uci_Loop(BoardState *pos, Search *info) {
    
    info->GAME_MODE = UCIMODE;
    
    // turn off buffering
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    // make line w input buffer available. Print data in uci format
    char line[INPUTBUFFER];
    printf("id name %s\n",NAME);
    printf("id author Sam Acquaviva\n");
    printf("option name Hash type spin default 64 min 4 max %d\n",MAX_HASH);
    printf("uciok\n");
    
    int MB = 64;
    
    // infinite loop, only broken when told to quit
    while (TRUE) {
        // clear first line and flush console
        memset(&line[0], 0, sizeof(line));
        fflush(stdout);
        
        // get input, if nothing, then continue
        if (!fgets(line, INPUTBUFFER, stdin))
            continue;
        
        // if receive just new line, then continue
        if (line[0] == '\n')
            continue;
        
        // respond to GUI input
        if (!strncmp(line, "isready", 7)) {
            printf("readyok\n");
            continue;
        } else if (!strncmp(line, "position", 8)) {
            ParsePosition(line, pos);
        } else if (!strncmp(line, "ucinewgame", 10)) {
            ParsePosition("position startpos\n", pos);
        } else if (!strncmp(line, "go", 2)) {
            printf("Seen Go..\n");
            ParseGo(line, info, pos);
        } else if (!strncmp(line, "quit", 4)) {
            info->quit = TRUE;
            break;
        } else if (!strncmp(line, "uci", 3)) {
            printf("id name %s\n",NAME);
            printf("id author Bluefever\n");
            printf("uciok\n");
        } else if (!strncmp(line, "debug", 4)) {
            DebugBoardState(pos,info);
            break;
        } else if (!strncmp(line, "setoption name Hash value ", 26)) {
            sscanf(line,"%*s %*s %*s %*s %d",&MB);
            if(MB < 4) MB = 4;
            if(MB > MAX_HASH) MB = MAX_HASH;
            printf("Set Hash to %d MB\n",MB);
            InitHashTable(pos->hashTable, MB);
        }
        if(info->quit) break;
    }
    pos->hashTable->hashEntry = malloc(10);  // TODO: Allocate enough memory!!
    free(pos->hashTable->hashEntry);
}

// MARK: - XBoard

// checks for 3 fold repetition
int ThreeFoldRep(const BoardState *pos) {
    
    ASSERT(BoardListsConsistent(pos));
    
    int i = 0, r = 0;
    for (i = 0; i < pos->hisPly; ++i)    {
        if (pos->history[i].hashKey == pos->hashKey) {
            r++;
        }
    }
    return r;
}

// checks for insufficient material to mate
int DrawMaterial(const BoardState *pos) {
    ASSERT(BoardListsConsistent(pos));
    
    if (pos->pceNum[wP] || pos->pceNum[bP]) return FALSE;
    if (pos->pceNum[wQ] || pos->pceNum[bQ] || pos->pceNum[wR] || pos->pceNum[bR]) return FALSE;
    if (pos->pceNum[wB] > 1 || pos->pceNum[bB] > 1) {return FALSE;}
    if (pos->pceNum[wN] > 1 || pos->pceNum[bN] > 1) {return FALSE;}
    if (pos->pceNum[wN] && pos->pceNum[wB]) {return FALSE;}
    if (pos->pceNum[bN] && pos->pceNum[bB]) {return FALSE;}
    
    return TRUE;
}

int checkresult(BoardState *pos) {
    ASSERT(BoardListsConsistent(pos));
    
    if (pos->fiftyMove > 100) {
        printf("1/2-1/2 {fifty move rule (claimed by SECS)}\n"); return TRUE;
    }
    
    if (ThreeFoldRep(pos) >= 2) {
        printf("1/2-1/2 {3-fold repetition (claimed by SECS)}\n"); return TRUE;
    }
    
    if (DrawMaterial(pos) == TRUE) {
        printf("1/2-1/2 {insufficient material (claimed by SECS)}\n"); return TRUE;
    }
    
    MoveList list[1];
    GenerateAllMoves(pos,list);
    
    int MoveNum = 0;
    int found = 0;
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        
        // if illegal move, continue
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        found++;
        TakeBackMove(pos);
        break;
    }
    
    // not checkmate or stalemate
    if(found != 0) return FALSE;
    
    // claim either stalemate or checkmate
    int InCheck = IsSqAttacked(pos->KingSq[pos->side],pos->side^1,pos);
    if(InCheck == TRUE)    {
        if(pos->side == WHITE) {
            printf("0-1 {black mates (claimed by SECS)}\n");return TRUE;
        } else {
            printf("0-1 {white mates (claimed by SECS)}\n");return TRUE;
        }
    } else {
        printf("\n1/2-1/2 {stalemate (claimed by SECS)}\n");return TRUE;
    }
    return FALSE;
}

void PrintOptions() {
    printf("feature ping=1 setboard=1 colors=0 usermove=1 memory=1\n");
    printf("feature done=1\n");
}

// input and output checking for xboard protocol
// from http://www.open-aurec.com/wbforum/viewtopic.php?f=24&t=51739
void XBoard_Loop(BoardState *pos, Search *info) {
    
    info->GAME_MODE = XBOARDMODE;
    info->POST_THINKING = TRUE;
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    PrintOptions(); // HACK
    
    int depth = -1, movestogo[2] = {30,30 }, movetime = -1;
    int time = -1, inc = 0;
    int engineSide = BOTH;
    int timeLeft;
    int sec;
    int mps;    // moves per session
    int move = NOMOVE;
    char inBuf[80], command[80];
    int MB;
    
    engineSide = BLACK;
    ParseFEN(START_FEN, pos);
    depth = -1;
    time = -1;
    
    while(TRUE) {
        
        fflush(stdout);
        
        // engine turn to move
        if(pos->side == engineSide && checkresult(pos) == FALSE) {
            info->starttime = GetTimeMs();
            info->depth = depth;
            
            // set time control
            if(time != -1) {
                info->timeset = TRUE;
                time /= movestogo[pos->side];
                time -= 50;
                info->stoptime = info->starttime + time + inc;
            }
            
            // don't limit depth
            if(depth == -1 || depth > MAXSEARCHDEPTH) {
                info->depth = MAXSEARCHDEPTH;
            }
            
            printf("time:%d start:%d stop:%d depth:%d timeset:%d movestogo:%d mps:%d\n",
                   time,info->starttime,info->stoptime,info->depth,info->timeset, movestogo[pos->side], mps);
            SearchPosition(pos, info);
            
            // use moves per session to figure out # moves left and decide how long to search
            if(mps != 0) {
                movestogo[pos->side^1]--;
                if(movestogo[pos->side^1] < 1) {
                    movestogo[pos->side^1] = mps;
                }
            }
            
        }
        
        fflush(stdout);
        
        memset(&inBuf[0], 0, sizeof(inBuf));
        fflush(stdout);
        if (!fgets(inBuf, 80, stdin))
            continue;
        
        sscanf(inBuf, "%s", command);
        
        printf("command seen:%s\n",inBuf);
        
        // quit
        if(!strcmp(command, "quit")) {
            info->quit = TRUE;
            break;
        }
        
        // tell engine not to think
        if(!strcmp(command, "force")) {
            engineSide = BOTH;
            continue;
        }
        
        // set all features we support in our engine
        if(!strcmp(command, "protover")){
            PrintOptions();
            continue;
        }
        
        // set search depth max
        if(!strcmp(command, "sd")) {
            sscanf(inBuf, "sd %d", &depth);
            printf("DEBUG depth:%d\n",depth);
            continue;
        }
        
        // set search time max
        if(!strcmp(command, "st")) {
            sscanf(inBuf, "st %d", &movetime);
            printf("DEBUG movetime:%d\n",movetime);
            continue;
        }
        
        if(!strcmp(command, "time")) {
            sscanf(inBuf, "time %d", &time);
            time *= 10;
            printf("DEBUG time:%d\n",time);
            continue;
        }
        
        if(!strcmp(command, "memory")) {
            sscanf(inBuf, "memory %d", &MB);
            if(MB < 4) MB = 4;
            if(MB > MAX_HASH) MB = MAX_HASH;
            printf("Set Hash to %d MB\n",MB);
            InitHashTable(pos->hashTable, MB);
            continue;
        }
        
        if(!strcmp(command, "level")) {
            sec = 0;
            movetime = -1;
            if( sscanf(inBuf, "level %d %d %d", &mps, &timeLeft, &inc) != 3) {
                sscanf(inBuf, "level %d %d:%d %d", &mps, &timeLeft, &sec, &inc);
                printf("DEBUG level with :\n");
            }    else {
                printf("DEBUG level without :\n");
            }
            timeLeft *= 60000;
            timeLeft += sec * 1000;
            movestogo[0] = movestogo[1] = 30;
            if(mps != 0) {
                movestogo[0] = movestogo[1] = mps;
            }
            time = -1;
            printf("DEBUG level timeLeft:%d movesToGo:%d inc:%d mps%d\n",timeLeft,movestogo[0],inc,mps);
            continue;
        }
        
        // expects 'pong' reply
        if(!strcmp(command, "ping")) {
            printf("pong%s\n", inBuf+4);
            continue;
        }
        
        // restart game
        if(!strcmp(command, "new")) {
            ClearHashTable(pos->hashTable);
            engineSide = BLACK; // part of protocol
            ParseFEN(START_FEN, pos);
            depth = -1;
            time = -1;
            continue;
        }
        
        // set board to position
        if(!strcmp(command, "setboard")){
            engineSide = BOTH;
            ParseFEN(inBuf+9, pos);
            continue;
        }
        
        // start thinking
        if(!strcmp(command, "go")) {
            engineSide = pos->side;
            continue;
        }
        
        // other player made move
        if(!strcmp(command, "usermove")){
            movestogo[pos->side]--;
            move = ParseMove(inBuf+9, pos);
            if(move == NOMOVE) continue;
            MakeMove(pos, move);
            pos->ply=0;
        }
    }
}

// MARK: - Console

// play against engine in console
void Console_Loop(BoardState *pos, Search *info) {
    
    printf("Welcome to SECS In Console Mode!\n");
    printf("Type help for commands\n\n");
    
    info->GAME_MODE = CONSOLEMODE;
    info->POST_THINKING = TRUE;
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    int depth = MAXSEARCHDEPTH, movetime = 3000;
    int engineSide = BOTH;
    int move = NOMOVE;
    char inBuf[80], command[80];
    
    engineSide = BLACK;
    ParseFEN(START_FEN, pos);
    
    while(TRUE) {
        
        fflush(stdout);
        
        if(pos->side == engineSide && checkresult(pos) == FALSE) {
            info->starttime = GetTimeMs();
            info->depth = depth;
            
            if(movetime != 0) {
                info->timeset = TRUE;
                info->stoptime = info->starttime + movetime;
            }
            
            SearchPosition(pos, info);
        }
        
        printf("\nSECS > ");
        
        fflush(stdout);
        
        memset(&inBuf[0], 0, sizeof(inBuf));
        fflush(stdout);
        if (!fgets(inBuf, 80, stdin))
            continue;
        
        sscanf(inBuf, "%s", command);
        
        if(!strcmp(command, "help")) {
            printf("Commands:\n");
            printf("quit - quit game\n");
            printf("force - computer will not think\n");
            printf("print - show board\n");
            printf("post - show thinking\n");
            printf("nopost - do not show thinking\n");
            printf("new - start new game\n");
            printf("go - set computer thinking\n");
            printf("depth x - set depth to x\n");
            printf("time x - set thinking time to x seconds (depth still applies if set)\n");
            printf("view - show current depth and movetime settings\n");
            printf("setboard x - set position to fen x\n");
            printf("** note ** - to reset time and depth, set to 0\n");
            printf("enter moves using b7b8q notation\n\n\n");
            continue;
        }
        
        if(!strcmp(command, "mirror")) {
            engineSide = BOTH;
            MirrorEvalTest(pos);
            continue;
        }
        
        if(!strcmp(command, "eval")) {
            PrintBoard(pos);
            printf("Eval:%d",PositionEvaluation(pos));
            MirrorBoard(pos);
            PrintBoard(pos);
            printf("Eval:%d",PositionEvaluation(pos));
            continue;
        }
        
        if(!strcmp(command, "setboard")){
            engineSide = BOTH;
            ParseFEN(inBuf+9, pos);
            continue;
        }
        
        if(!strcmp(command, "quit")) {
            info->quit = TRUE;
            break;
        }
        
        if(!strcmp(command, "post")) {
            info->POST_THINKING = TRUE;
            continue;
        }
        
        if(!strcmp(command, "print")) {
            PrintBoard(pos);
            continue;
        }
        
        if(!strcmp(command, "nopost")) {
            info->POST_THINKING = FALSE;
            continue;
        }
        
        if(!strcmp(command, "force")) {
            engineSide = BOTH;
            continue;
        }
        
        if(!strcmp(command, "view")) {
            if(depth == MAXSEARCHDEPTH) printf("depth not set ");
            else printf("depth %d",depth);
            
            if(movetime != 0) printf(" movetime %ds\n",movetime/1000);
            else printf(" movetime not set\n");
            
            continue;
        }
        
        if(!strcmp(command, "depth")) {
            sscanf(inBuf, "depth %d", &depth);
            if(depth==0) depth = MAXSEARCHDEPTH;
            continue;
        }
        
        if(!strcmp(command, "time")) {
            sscanf(inBuf, "time %d", &movetime);
            movetime *= 1000;
            continue;
        }
        
        if(!strcmp(command, "new")) {
            ClearHashTable(pos->hashTable);
            engineSide = BLACK;
            ParseFEN(START_FEN, pos);
            continue;
        }
        
        if(!strcmp(command, "go")) {
            engineSide = pos->side;
            continue;
        }
        
        move = ParseMove(inBuf, pos);
        if(move == NOMOVE) {
            printf("Command unknown:%s\n",inBuf);
            continue;
        }
        MakeMove(pos, move);
        pos->ply=0;
    }
}

// print square in algebraic notation
char *PrSq(const int sq) {
    
    static char SqStr[3];
    
    int file = FilesBrd[sq];
    int rank = RanksBrd[sq];
    
    sprintf(SqStr, "%c%c", ('a'+file), ('1'+rank));
    
    return SqStr;
}

// print move in algebraic notation
char *PrMove(const int move) {
    
    static char MvStr[6];
    
    int ff = FilesBrd[FROMSQ(move)];
    int rf = RanksBrd[FROMSQ(move)];
    int ft = FilesBrd[TOSQ(move)];
    int rt = RanksBrd[TOSQ(move)];
    
    int promoted = PROMOTED(move);
    
    if(promoted) {
        char pchar = 'q';
        if(IsKn(promoted)) {
            pchar = 'n';
        } else if(IsRQ(promoted) && !IsBQ(promoted))  {
            pchar = 'r';
        } else if(!IsRQ(promoted) && IsBQ(promoted))  {
            pchar = 'b';
        }
        sprintf(MvStr, "%c%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt), pchar);
    } else {
        sprintf(MvStr, "%c%c%c%c", ('a'+ff), ('1'+rf), ('a'+ft), ('1'+rt));
    }
    
    return MvStr;
}

int ParseMove(char *ptrChar, BoardState *pos) {
    
    ASSERT(BoardListsConsistent(pos));
    
    // ensuring of form ex: a2b3
    if(ptrChar[1] > '8' || ptrChar[1] < '1') return NOMOVE;
    if(ptrChar[3] > '8' || ptrChar[3] < '1') return NOMOVE;
    if(ptrChar[0] > 'h' || ptrChar[0] < 'a') return NOMOVE;
    if(ptrChar[2] > 'h' || ptrChar[2] < 'a') return NOMOVE;
    
    int from = FR2SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
    int to = FR2SQ(ptrChar[2] - 'a', ptrChar[3] - '1');
    
    ASSERT(SqOnBoard(from) && SqOnBoard(to));
    
    MoveList list[1];
    GenerateAllMoves(pos,list);
    int MoveNum = 0;
    int Move = 0;
    int PromPce = EMPTY;
    
    // loop through all moves in move list and make that move
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        Move = list->moves[MoveNum].move;
        if(FROMSQ(Move)==from && TOSQ(Move)==to) {
            // if promotion, promote piece
            PromPce = PROMOTED(Move);
            if(PromPce!=EMPTY) {
                if(IsRQ(PromPce) && !IsBQ(PromPce) && ptrChar[4]=='r') {
                    return Move;
                } else if(!IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4]=='b') {
                    return Move;
                } else if(IsRQ(PromPce) && IsBQ(PromPce) && ptrChar[4]=='q') {
                    return Move;
                } else if(IsKn(PromPce)&& ptrChar[4]=='n') {
                    return Move;
                }
                continue;
            }
            return Move;
        }
    }
    return NOMOVE;
}

// walks through move list and prints out each move
void PrintMoveList(const MoveList *list) {
    int index = 0;
    int score = 0;
    int move = 0;
    printf("MoveList:\n");
    
    for(index = 0; index < list->count; ++index) {
        
        move = list->moves[index].move;
        score = list->moves[index].score;
        
        printf("Move:%d > %s (score:%d)\n",index+1,PrMove(move),score);
    }
    printf("MoveList Total %d Moves:\n\n",list->count);
}
