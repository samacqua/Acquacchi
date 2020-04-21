// validate.c

#include "stdio.h"
#include "string.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

// if returns TRUE, then valid.

// checking all lists and current board state are consistent
int BoardListsConsistent(const BoardState *pos) {
    
    int t_pceNum[13] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int t_bigPce[2] = { 0, 0};
    int t_majPce[2] = { 0, 0};
    int t_minPce[2] = { 0, 0};
    int t_material[2] = { 0, 0};
    
    int sq64,t_piece,t_pce_num,sq120,color,pcount;
    
    U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};
    
    t_pawns[WHITE] = pos->pawns[WHITE];
    t_pawns[BLACK] = pos->pawns[BLACK];
    t_pawns[BOTH] = pos->pawns[BOTH];
    
    // check piece lists
    for(t_piece = wP; t_piece <= bK; ++t_piece) {
        for(t_pce_num = 0; t_pce_num < pos->pceNum[t_piece]; ++t_pce_num) {
            sq120 = pos->pieceList[t_piece][t_pce_num];
            ASSERT(pos->pieces[sq120]==t_piece);
        }
    }
    
    // check piece count and other counters
    for(sq64 = 0; sq64 < 64; ++sq64) {
        sq120 = SQ120(sq64);
        t_piece = pos->pieces[sq120];
        t_pceNum[t_piece]++;
        color = PieceCol[t_piece];
        if( IsPieceBig[t_piece] == TRUE) t_bigPce[color]++;
        if( IsPieceMin[t_piece] == TRUE) t_minPce[color]++;
        if( IsPieceBig[t_piece] == TRUE) t_majPce[color]++;
        
        t_material[color] += PieceVal[t_piece];
    }
    
    for(t_piece = wP; t_piece <= bK; ++t_piece) {
        ASSERT(t_pceNum[t_piece]==pos->pceNum[t_piece]);
    }
    
    // check bitboards count
    pcount = CNT(t_pawns[WHITE]);
    ASSERT(pcount == pos->pceNum[wP]);
    pcount = CNT(t_pawns[BLACK]);
    ASSERT(pcount == pos->pceNum[bP]);
    pcount = CNT(t_pawns[BOTH]);
    ASSERT(pcount == (pos->pceNum[bP] + pos->pceNum[wP]));
    
    // check bitboards squares
    while(t_pawns[WHITE]) {
        sq64 = POP(&t_pawns[WHITE]);
        ASSERT(pos->pieces[SQ120(sq64)] == wP);
    }
    
    while(t_pawns[BLACK]) {
        sq64 = POP(&t_pawns[BLACK]);
        ASSERT(pos->pieces[SQ120(sq64)] == bP);
    }
    
    while(t_pawns[BOTH]) {
        sq64 = POP(&t_pawns[BOTH]);
        ASSERT( (pos->pieces[SQ120(sq64)] == bP) || (pos->pieces[SQ120(sq64)] == wP) );
    }
    
    ASSERT(t_material[WHITE]==pos->material[WHITE] && t_material[BLACK]==pos->material[BLACK]);
    ASSERT(t_minPce[WHITE]==pos->minPce[WHITE] && t_minPce[BLACK]==pos->minPce[BLACK]);
    ASSERT(t_majPce[WHITE]==pos->majPce[WHITE] && t_majPce[BLACK]==pos->majPce[BLACK]);
    ASSERT(t_bigPce[WHITE]==pos->bigPce[WHITE] && t_bigPce[BLACK]==pos->bigPce[BLACK]);
    
    ASSERT(pos->side==WHITE || pos->side==BLACK);
    ASSERT(GenerateHashKey(pos)==pos->hashKey);
    
    ASSERT(pos->enPas==NO_SQ || ( RanksBrd[pos->enPas]==RANK_6 && pos->side == WHITE)
           || ( RanksBrd[pos->enPas]==RANK_3 && pos->side == BLACK));
    
    ASSERT(pos->pieces[pos->KingSq[WHITE]] == wK);
    ASSERT(pos->pieces[pos->KingSq[BLACK]] == bK);
    
    ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);
    
    ASSERT(PieceListValid(pos));
    
    return TRUE;
}

int PieceListValid(const BoardState *pos) {
    int pce = wP;
    int sq;
    int num;
    for(pce = wP; pce <= bK; ++pce) {
        if(pos->pceNum[pce]<0 || pos->pceNum[pce]>=10) return FALSE;
    }
    
    if(pos->pceNum[wK]!=1 || pos->pceNum[bK]!=1) return FALSE;
    
    for(pce = wP; pce <= bK; ++pce) {
        for(num = 0; num < pos->pceNum[pce]; ++num) {
            sq = pos->pieceList[pce][num];
            if(!SqOnBoard(sq)) return FALSE;
        }
    }
    return TRUE;
}

int MoveListValid(const MoveList *list,  const BoardState *pos) {
    if(list->count < 0 || list->count >= MAXPOSITIONMOVES) {
        return FALSE;
    }
    
    int MoveNum;
    int from = 0;
    int to = 0;
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        to = TOSQ(list->moves[MoveNum].move);
        from = FROMSQ(list->moves[MoveNum].move);
        if(!SqOnBoard(to) || !SqOnBoard(from)) {
            return FALSE;
        }
        if(!PieceValid(pos->pieces[from])) {
            PrintBoard(pos);
            return FALSE;
        }
    }
    return TRUE;
}

int SqIs120(const int sq) {
    return (sq>=0 && sq<120);
}

int PieceEmptyOrOffbrd(const int pce) {
    return (PieceEmpty(pce) || pce == OFFBOARD);
}
int SqOnBoard(const int sq) {
    return FilesBrd[sq]==OFFBOARD ? 0 : 1;
}

int SideValid(const int side) {
    return (side==WHITE || side == BLACK) ? 1 : 0;
}

int FileRankValid(const int fr) {
    return (fr >= 0 && fr <= 7) ? 1 : 0;
}

int PieceEmpty(const int pce) {
    return (pce >= EMPTY && pce <= bK) ? 1 : 0;
}

int PieceValid(const int pce) {
    return (pce >= wP && pce <= bK) ? 1 : 0;
}

void DebugBoardState(BoardState *pos, Search *info) {
    
    FILE *file;
    file = fopen("lct2.epd","r");
    char lineIn [1024];
    
    info->depth = MAXSEARCHDEPTH;
    info->timeset = TRUE;
    int time = 1140000;
    
    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        while(fgets (lineIn , 1024 , file) != NULL) {
            info->starttime = GetTimeMs();
            info->stoptime = info->starttime + time;
            ClearHashTable(pos->hashTable);
            ParseFEN(lineIn, pos);
            printf("\n%s\n",lineIn);
            printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
                   time,info->starttime,info->stoptime,info->depth,info->timeset);
            SearchPosition(pos, info);
            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}

void MirrorEvalTest(BoardState *pos) {
    FILE *file;
    file = fopen("mirror.epd","r");
    char lineIn [1024];
    int ev1 = 0; int ev2 = 0;
    int positions = 0;
    if(file == NULL) {
        printf("File Not Found\n");
        return;
    }  else {
        while(fgets (lineIn , 1024 , file) != NULL) {
            ParseFEN(lineIn, pos);
            positions++;
            ev1 = PositionEvaluation(pos);
            MirrorBoard(pos);
            ev2 = PositionEvaluation(pos);
            
            if(ev1 != ev2) {
                printf("\n\n\n");
                ParseFEN(lineIn, pos);
                PrintBoard(pos);
                MirrorBoard(pos);
                PrintBoard(pos);
                printf("\n\nMirror Fail:\n%s\n",lineIn);
                getchar();
                return;
            }
            
            if( (positions % 1000) == 0)   {
                printf("position %d\n",positions);
            }
            
            memset(&lineIn[0], 0, sizeof(lineIn));
        }
    }
}

