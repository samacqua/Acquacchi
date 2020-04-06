// movegen.c

#include "stdio.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

// from, to, capture, promote, flag
#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
#define SQOFFBOARD(sq) (FilesBrd[(sq)]==OFFBOARD)

// directions in 120 square board that each piece can move
const int KnDir[8] = { -8, -19,    -21, -12, 8, 19, 21, 12 };
const int RkDir[4] = { -1, -10,    1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDir[8] = { -1, -10,    1, 10, -9, -11, 11, 9 };

// returns if a given square is attcked
int SqAttacked(const int sq, const int side, const BoardState *pos) {
    
    int pce,index,t_sq,dir;
    
    ASSERT(SqOnBoard(sq));
    ASSERT(SideValid(side));
    ASSERT(CheckBoard(pos));
    
    // pawns
    if(side == WHITE) {
        if(pos->pieces[sq-11] == wP || pos->pieces[sq-9] == wP) {
            return TRUE;
        }
    } else {
        if(pos->pieces[sq+11] == bP || pos->pieces[sq+9] == bP) {
            return TRUE;
        }
    }
    
    // knights
    for(index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KnDir[index]];
        ASSERT(PceValidEmptyOffbrd(pce));
        if(pce != OFFBOARD && IsKn(pce) && PieceCol[pce]==side) {
            return TRUE;
        }
    }
    
    // rooks, queens
    // loop through all squares in each of four cardinal directions (direction that rook and queen can move) until there is a piece or off the board. if piece, check if rook of opposite color. if so, attacked.
    for(index = 0; index < 4; ++index) {
        dir = RkDir[index];
        t_sq = sq + dir;
        ASSERT(SqIs120(t_sq));
        pce = pos->pieces[t_sq];
        ASSERT(PceValidEmptyOffbrd(pce));
        while(pce != OFFBOARD) {
            if(pce != EMPTY) {
                if(IsRQ(pce) && PieceCol[pce] == side) {
                    return TRUE;
                }
                break;
            }
            t_sq += dir;
            ASSERT(SqIs120(t_sq));
            pce = pos->pieces[t_sq];
        }
    }
    
    // bishops, queens
    // same as for rooks and queens, but diagonals
    for(index = 0; index < 4; ++index) {
        dir = BiDir[index];
        t_sq = sq + dir;
        ASSERT(SqIs120(t_sq));
        pce = pos->pieces[t_sq];
        ASSERT(PceValidEmptyOffbrd(pce));
        while(pce != OFFBOARD) {
            if(pce != EMPTY) {
                if(IsBQ(pce) && PieceCol[pce] == side) {
                    return TRUE;
                }
                break;
            }
            t_sq += dir;
            ASSERT(SqIs120(t_sq));
            pce = pos->pieces[t_sq];
        }
    }
    
    // kings
    for(index = 0; index < 8; ++index) {
        pce = pos->pieces[sq + KiDir[index]];
        ASSERT(PceValidEmptyOffbrd(pce));
        if(pce != OFFBOARD && IsKi(pce) && PieceCol[pce]==side) {
            return TRUE;
        }
    }
    
    return FALSE;
    
}

// loop through array until result is 0, so will loop for each sliding piece for whichever side it is to move
const int LoopSlidePce[8] = {
    wB, wR, wQ, 0, bB, bR, bQ, 0
};
const int LoopNonSlidePce[6] = {
    wN, wK, 0, bN, bK, 0
};

// if white, start looping thru LoopSlidePce at 0, if black then 4
const int LoopSlideIndex[2] = { 0, 4 };
const int LoopNonSlideIndex[2] = { 0, 3 };

const int PceDir[13][8] = {
    { 0, 0, 0, 0, 0, 0, 0 },                // empty
    { 0, 0, 0, 0, 0, 0, 0 },                // wP
    { -8, -19,	-21, -12, 8, 19, 21, 12 },  // wN
    { -9, -11, 11, 9, 0, 0, 0, 0 },         // wB
    { -1, -10,	1, 10, 0, 0, 0, 0 },        // wR
    { -1, -10,	1, 10, -9, -11, 11, 9 },    // wQ
    { -1, -10,	1, 10, -9, -11, 11, 9 },    // wK
    { 0, 0, 0, 0, 0, 0, 0 },                // bP
    { -8, -19,	-21, -12, 8, 19, 21, 12 },  // bN
    { -9, -11, 11, 9, 0, 0, 0, 0 },         // bB
    { -1, -10,	1, 10, 0, 0, 0, 0 },        // bR
    { -1, -10,	1, 10, -9, -11, 11, 9 },    // bQ
    { -1, -10,	1, 10, -9, -11, 11, 9 }     //bK
};

// number of directions in PceDir for each piece type
const int NumDir[13] = {
    0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

/*
 Move search order: (https://www.chessprogramming.org/Move_Ordering)
 1. PV Move         Score: 2000000
 2. Cap -> MvvLVA   Score: 1000000 - 1000606
 3. Killers         Score: 800000 - 900000
 4. HistoryScore    Score: 0 +1 increments
 */
const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

void InitMvvLva() {
    int Attacker;
    int Victim;
    // loop through all attacker/victim combinations and set the MvvLva score
    for(Attacker = wP; Attacker <= bK; ++Attacker) {
        for(Victim = wP; Victim <= bK; ++Victim) {
            // ex: pawn takes queen = 506 - 1, knight takes queen is 506-2, so pawn takes queen has higher score and will be searched first
            MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - ( VictimScore[Attacker] / 100);
        }
    }
}

// check if move exists, because when probing hash table, might be unlucky and get two of the same indexes, so a nonmove might be introduced.
int MoveExists(BoardState *pos, const int move) {
    
    MoveList list[1];
    GenerateAllMoves(pos,list);
    
    int MoveNum = 0;
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        TakeMove(pos);
        if(list->moves[MoveNum].move == move) {
            return TRUE;
        }
    }
    return FALSE;
}

// add a non-capture move to a movelist and add move score (used for search order)
static void AddQuietMove( const BoardState *pos, int move, MoveList *list ) {
    
    ASSERT(SqOnBoard(FROMSQ(move)));
    ASSERT(SqOnBoard(TOSQ(move)));
    ASSERT(CheckBoard(pos));
    ASSERT(pos->ply >=0 && pos->ply < MAXSEARCHDEPTH);
    
    list->moves[list->count].move = move;
    
    // set score of search heuristics
    if(pos->searchKillers[0][pos->ply] == move) {
        list->moves[list->count].score = 900000;
    } else if(pos->searchKillers[1][pos->ply] == move) {
        list->moves[list->count].score = 800000;
    } else {
        list->moves[list->count].score = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];
    }
    list->count++;
}

// add a capture move to a movelist and add move score (used for search order)
static void AddCaptureMove( const BoardState *pos, int move, MoveList *list ) {
    
    ASSERT(SqOnBoard(FROMSQ(move)));
    ASSERT(SqOnBoard(TOSQ(move)));
    ASSERT(PieceValid(CAPTURED(move)));
    ASSERT(CheckBoard(pos));
    
    // compute mvvlva score
    list->moves[list->count].move = move;
    list->moves[list->count].score = MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 1000000;
    list->count++;
}

// add an en passant move to a movelist and add move score (used for search order)
static void AddEnPassantMove( const BoardState *pos, int move, MoveList *list ) {
    
    ASSERT(SqOnBoard(FROMSQ(move)));
    ASSERT(SqOnBoard(TOSQ(move)));
    ASSERT(CheckBoard(pos));
    ASSERT((RanksBrd[TOSQ(move)]==RANK_6 && pos->side == WHITE) || (RanksBrd[TOSQ(move)]==RANK_3 && pos->side == BLACK));
    
    // add mvvlva score
    list->moves[list->count].move = move;
    list->moves[list->count].score = 105 + 1000000; // pawn takes pawn is 105
    list->count++;
}

// pawns are tricky because of en passant and promotions, so handled in own function instead of looping through move direction array.
static void AddWhitePawnCapMove( const BoardState *pos, const int from, const int to, const int cap, MoveList *list ) {
    
    ASSERT(PieceValidEmpty(cap));
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(CheckBoard(pos));
    
    // promotion
    if(RanksBrd[from] == RANK_7) {
        AddCaptureMove(pos, MOVE(from,to,cap,wQ,0), list);
        AddCaptureMove(pos, MOVE(from,to,cap,wR,0), list);
        AddCaptureMove(pos, MOVE(from,to,cap,wB,0), list);
        AddCaptureMove(pos, MOVE(from,to,cap,wN,0), list);
    } else {
        AddCaptureMove(pos, MOVE(from,to,cap,EMPTY,0), list);
    }
}

static void AddWhitePawnMove( const BoardState *pos, const int from, const int to, MoveList *list ) {
    
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(CheckBoard(pos));
    
    // promotion
    if(RanksBrd[from] == RANK_7) {
        AddQuietMove(pos, MOVE(from,to,EMPTY,wQ,0), list);
        AddQuietMove(pos, MOVE(from,to,EMPTY,wR,0), list);
        AddQuietMove(pos, MOVE(from,to,EMPTY,wB,0), list);
        AddQuietMove(pos, MOVE(from,to,EMPTY,wN,0), list);
    } else {
        AddQuietMove(pos, MOVE(from,to,EMPTY,EMPTY,0), list);
    }
}

static void AddBlackPawnCapMove( const BoardState *pos, const int from, const int to, const int cap, MoveList *list ) {
    
    ASSERT(PieceValidEmpty(cap));
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(CheckBoard(pos));
    
    // promotion
    if(RanksBrd[from] == RANK_2) {
        AddCaptureMove(pos, MOVE(from,to,cap,bQ,0), list);
        AddCaptureMove(pos, MOVE(from,to,cap,bR,0), list);
        AddCaptureMove(pos, MOVE(from,to,cap,bB,0), list);
        AddCaptureMove(pos, MOVE(from,to,cap,bN,0), list);
    } else {
        AddCaptureMove(pos, MOVE(from,to,cap,EMPTY,0), list);
    }
}

static void AddBlackPawnMove( const BoardState *pos, const int from, const int to, MoveList *list ) {
    
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(CheckBoard(pos));
    
    // promotion
    if(RanksBrd[from] == RANK_2) {
        AddQuietMove(pos, MOVE(from,to,EMPTY,bQ,0), list);
        AddQuietMove(pos, MOVE(from,to,EMPTY,bR,0), list);
        AddQuietMove(pos, MOVE(from,to,EMPTY,bB,0), list);
        AddQuietMove(pos, MOVE(from,to,EMPTY,bN,0), list);
    } else {
        AddQuietMove(pos, MOVE(from,to,EMPTY,EMPTY,0), list);
    }
}

void GenerateAllMoves(const BoardState *pos, MoveList *list) {
    
    ASSERT(CheckBoard(pos));
    
    list->count = 0;
    
    int pce = EMPTY;
    int side = pos->side;
    int sq = 0; int t_sq = 0;
    int pceNum = 0;
    int dir = 0;
    int index = 0;
    int pceIndex = 0;
    
    if(side == WHITE) {
        // loop through all white pawns
        for(pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
            sq = pos->pieceList[wP][pceNum];
            ASSERT(SqOnBoard(sq));
            
            // add non-capture forward move
            if(pos->pieces[sq + 10] == EMPTY) {
                AddWhitePawnMove(pos, sq, sq+10, list);
                // add double move
                if(RanksBrd[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) {
                    AddQuietMove(pos, MOVE(sq,(sq+20),EMPTY,EMPTY,MFLAGPS),list);
                }
            }
            // add capture move
            if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
            }
            // add capture move on other diagonal
            if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
            }
            // add en passant moves
            if(pos->enPas != NO_SQ) {
                if(sq + 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,MFLAGEP), list);
                }
                if(sq + 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,MFLAGEP), list);
                }
            }
        }
        
        // add white kingside castle
        if(pos->castlePerm & WKCA) {
            if(pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
                if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(F1,BLACK,pos) ) {
                    AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
        
        // add white queenside  castle
        if(pos->castlePerm & WQCA) {
            if(pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
                if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(D1,BLACK,pos) ) {
                    AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
        
    } else if (side==BLACK) {
        
        // loop through all pawns
        for(pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
            sq = pos->pieceList[bP][pceNum];
            ASSERT(SqOnBoard(sq));
            
            // add non-capture forward move
            if(pos->pieces[sq - 10] == EMPTY) {
                AddBlackPawnMove(pos, sq, sq-10, list);
                // add double move
                if(RanksBrd[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
                    AddQuietMove(pos, MOVE(sq,(sq-20),EMPTY,EMPTY,MFLAGPS),list);
                }
            }
            
            // add capture
            if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
            }
            
            // add capture on other diagonal
            if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
            }
            
            // add en passant moves
            if(pos->enPas != NO_SQ) {
                if(sq - 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,MFLAGEP), list);
                }
                if(sq - 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,MFLAGEP), list);
                }
            }
        }
        
        // add black kingside castle
        if(pos->castlePerm &  BKCA) {
            if(pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
                if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(F8,WHITE,pos) ) {
                    AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
        
        // add black queenside castle
        if(pos->castlePerm &  BQCA) {
            if(pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
                if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(D8,WHITE,pos) ) {
                    AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, MFLAGCA), list);
                }
            }
        }
    }
    
    // Loop through sliding pieces
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++];
    while( pce != 0) {
        ASSERT(PieceValid(pce));
        
        for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pieceList[pce][pceNum];
            ASSERT(SqOnBoard(sq));
            
            for(index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;
                
                // go in direction until reach end of board
                while(!SQOFFBOARD(t_sq)) {
                    // if square has a piece, add capture if opposite color, break if same color
                    if(pos->pieces[t_sq] != EMPTY) {
                        if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
                    t_sq += dir;
                }
            }
        }
        pce = LoopSlidePce[pceIndex++];
    }
    
    // Loop through non sliding pieces
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];
    
    while( pce != 0) {
        ASSERT(PieceValid(pce));
        
        for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pieceList[pce][pceNum];
            ASSERT(SqOnBoard(sq));
            
            for(index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;
                
                // if square not on board, don't add to moves
                if(SQOFFBOARD(t_sq)) {
                    continue;
                }
                
                // if square has opposite color piece, add capture. If square has current color, don't add move.
                if(pos->pieces[t_sq] != EMPTY) {
                    if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
                AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
            }
        }
        pce = LoopNonSlidePce[pceIndex++];
    }
    ASSERT(MoveListOk(list,pos));
}

// same as generate all moves, but only captures
void GenerateAllCaps(const BoardState *pos, MoveList *list) {
    
    ASSERT(CheckBoard(pos));
    
    list->count = 0;
    
    int pce = EMPTY;
    int side = pos->side;
    int sq = 0; int t_sq = 0;
    int pceNum = 0;
    int dir = 0;
    int index = 0;
    int pceIndex = 0;
    
    if(side == WHITE) {
        
        for(pceNum = 0; pceNum < pos->pceNum[wP]; ++pceNum) {
            sq = pos->pieceList[wP][pceNum];
            ASSERT(SqOnBoard(sq));
            
            if(!SQOFFBOARD(sq + 9) && PieceCol[pos->pieces[sq + 9]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
            }
            if(!SQOFFBOARD(sq + 11) && PieceCol[pos->pieces[sq + 11]] == BLACK) {
                AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
            }
            
            if(pos->enPas != NO_SQ) {
                if(sq + 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,MFLAGEP), list);
                }
                if(sq + 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,MFLAGEP), list);
                }
            }
        }
        
    } else {
        
        for(pceNum = 0; pceNum < pos->pceNum[bP]; ++pceNum) {
            sq = pos->pieceList[bP][pceNum];
            ASSERT(SqOnBoard(sq));
            
            if(!SQOFFBOARD(sq - 9) && PieceCol[pos->pieces[sq - 9]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
            }
            
            if(!SQOFFBOARD(sq - 11) && PieceCol[pos->pieces[sq - 11]] == WHITE) {
                AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
            }
            if(pos->enPas != NO_SQ) {
                if(sq - 9 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,MFLAGEP), list);
                }
                if(sq - 11 == pos->enPas) {
                    AddEnPassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,MFLAGEP), list);
                }
            }
        }
    }
    
    // Loop through slide pieces
    pceIndex = LoopSlideIndex[side];
    pce = LoopSlidePce[pceIndex++];
    while( pce != 0) {
        ASSERT(PieceValid(pce));
        
        for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pieceList[pce][pceNum];
            ASSERT(SqOnBoard(sq));
            
            for(index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;
                
                while(!SQOFFBOARD(t_sq)) {
                    // BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
                    if(pos->pieces[t_sq] != EMPTY) {
                        if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                            AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                        }
                        break;
                    }
                    t_sq += dir;
                }
            }
        }
        
        pce = LoopSlidePce[pceIndex++];
    }
    
    // Loop through non slide
    pceIndex = LoopNonSlideIndex[side];
    pce = LoopNonSlidePce[pceIndex++];
    
    while( pce != 0) {
        ASSERT(PieceValid(pce));
        
        for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
            sq = pos->pieceList[pce][pceNum];
            ASSERT(SqOnBoard(sq));
            
            for(index = 0; index < NumDir[pce]; ++index) {
                dir = PceDir[pce][index];
                t_sq = sq + dir;
                
                if(SQOFFBOARD(t_sq)) {
                    continue;
                }
                
                // BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
                if(pos->pieces[t_sq] != EMPTY) {
                    if( PieceCol[pos->pieces[t_sq]] == (side ^ 1)) {
                        AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
                    }
                    continue;
                }
            }
        }
        
        pce = LoopNonSlidePce[pceIndex++];
    }
    ASSERT(MoveListOk(list,pos));
}
