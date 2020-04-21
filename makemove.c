// makemove.c

#include "stdio.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

#define HASH_PCE(pce,sq) (pos->hashKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->hashKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->hashKey ^= (SideKey))
#define HASH_EP (pos->hashKey ^= (PieceKeys[EMPTY][(pos->enPas)]))

// ca_perm &= CastlePermissionsTable[from]
// 1111 == 15
// if move black king, 1111 & 3 = 1100 (no black castling rights now). Similar for moving white king.
// similar for moving rooks where if moved then that sides castling permission is lost via this bitwise addition
const int CastlePermissionsTable[120] = {   // all are 15 except A1, E1, H1, A8, E8, H8.
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

// clears the piece for a given square and position
static void ClearPiece(const int sq, BoardState *pos) {
    
    ASSERT(SqOnBoard(sq));
    ASSERT(BoardListsConsistent(pos));
    
    int pce = pos->pieces[sq];
    
    ASSERT(PieceValid(pce));
    
    int col = PieceCol[pce];
    int index = 0;
    int t_pceNum = -1;
    
    ASSERT(SideValid(col));
    
    // hash piece out of key
    HASH_PCE(pce,sq);
    
    // update piece lists and pawn bitboards
    pos->pieces[sq] = EMPTY;
    pos->material[col] -= PieceVal[pce];
    if(IsPieceBig[pce]) {
        pos->bigPce[col]--;
        if(IsPieceBig[pce]) {
            pos->majPce[col]--;
        } else {
            pos->minPce[col]--;
        }
    } else {
        CLRBIT(pos->pawns[col],SQ64(sq));
        CLRBIT(pos->pawns[BOTH],SQ64(sq));
    }
    
    // loop through piece index until we find the piece in the piece list that is on the square.
    /*
     if 5 white pawns, then pos->pceNum[wP] == 5, so looping through 0 to 4.
     pos->pieceList[wP][0] == sq0
     pos->pieceList[wP][1] == sq1
     pos->pieceList[wP][2] == sq2
     pos->pieceList[wP][3] == sq3
     pos->pieceList[wP][4] == sq4
     
     say sq is sq3 :
     sq==sq3 so t_pcNum = 3;
     */
    for(index = 0; index < pos->pceNum[pce]; ++index) {
        if(pos->pieceList[pce][index] == sq) {
            t_pceNum = index;
            break;
        }
    }
    
    ASSERT(t_pceNum != -1);
    ASSERT(t_pceNum>=0&&t_pceNum<10);
    
    pos->pceNum[pce]--;		// decrement number of pieces
    
    pos->pieceList[pce][t_pceNum] = pos->pieceList[pce][pos->pceNum[pce]];  // since decremented, effectively removed the chosen square
}

// add a piece to a given square in a given postion
static void SetPiece(const int sq, BoardState *pos, const int pce) {
    
    ASSERT(PieceValid(pce));
    ASSERT(SqOnBoard(sq));
    
    int col = PieceCol[pce];
    ASSERT(SideValid(col));
    
    // hash piece into hash key
    HASH_PCE(pce,sq);
    
    pos->pieces[sq] = pce;
    
    // increment cateogry counts and update pawn bitboards
    if(IsPieceBig[pce]) {
        pos->bigPce[col]++;
        if(IsPieceBig[pce]) {
            pos->majPce[col]++;
        } else {
            pos->minPce[col]++;
        }
    } else {
        SETBIT(pos->pawns[col],SQ64(sq));
        SETBIT(pos->pawns[BOTH],SQ64(sq));
    }
    
    pos->material[col] += PieceVal[pce];
    
    // add position to piece list, then increment the piece number
    pos->pieceList[pce][pos->pceNum[pce]++] = sq;
    
}

// move piece from one square to another in a given position
static void MovePiece(const int from, const int to, BoardState *pos) {
    
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    
    int index = 0;
    int pce = pos->pieces[from];	
    int col = PieceCol[pce];
    ASSERT(SideValid(col));
    ASSERT(PieceValid(pce));
    
#ifdef DEBUG
    int t_PieceNum = FALSE;
#endif
    
    // hash piece out of from square and set square to empty
    HASH_PCE(pce,from);
    pos->pieces[from] = EMPTY;
    
    // hash into to square and set square to piece
    HASH_PCE(pce,to);
    pos->pieces[to] = pce;
    
    // update pawn bitboards
    if(!IsPieceBig[pce]) {
        CLRBIT(pos->pawns[col],SQ64(from));
        CLRBIT(pos->pawns[BOTH],SQ64(from));
        SETBIT(pos->pawns[col],SQ64(to));
        SETBIT(pos->pawns[BOTH],SQ64(to));		
    }    
    
    for(index = 0; index < pos->pceNum[pce]; ++index) {
        if(pos->pieceList[pce][index] == from) {
            pos->pieceList[pce][index] = to;
#ifdef DEBUG
            t_PieceNum = TRUE;
#endif
            break;
        }
    }
    ASSERT(t_PieceNum);
}

// make a move given a move and a board
// takes care of move outside of just moving piece directly (as movepiece does)
// returns int, if 0 then moving into check so not valid move
int MakeMove(BoardState *pos, int move) {
    
    ASSERT(BoardListsConsistent(pos));
    
    int from = FROMSQ(move);
    int to = TOSQ(move);
    int side = pos->side;
    
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(SideValid(side));
    ASSERT(PieceValid(pos->pieces[from]));
    ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXTOTALMOVES);
    ASSERT(pos->ply >= 0 && pos->ply < MAXSEARCHDEPTH);
    
    // set history key to current position hash key (setting the last move for the next move)
    pos->history[pos->hisPly].hashKey = pos->hashKey;
    
    // if was en passant capture, clear square in front of en passant square (movepiece does not account for en passant)
    // if castle, move rook (movepiece only moves king)
    if(move & MFLAGEP) {
        if(side == WHITE) {
            ClearPiece(to-10,pos);
        } else {
            ClearPiece(to+10,pos);
        }
    } else if (move & MFLAGCA) {
        switch(to) {
            case C1:
                MovePiece(A1, D1, pos);
                break;
            case C8:
                MovePiece(A8, D8, pos);
                break;
            case G1:
                MovePiece(H1, F1, pos);
                break;
            case G8:
                MovePiece(H8, F8, pos);
                break;
            default: ASSERT(FALSE); break;
        }
    }	
    
    // hash out en passant square and castling permissions (will hash in updated en passant square and castling permissions for move later)
    if(pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;
    
    pos->history[pos->hisPly].move = move;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;
    
    // update castle permissions and reset en passant square
    pos->castlePerm &= CastlePermissionsTable[from];
    pos->castlePerm &= CastlePermissionsTable[to];
    pos->enPas = NO_SQ;
    
    // hash castling permissions back in
    HASH_CA;
    
    int captured = CAPTURED(move);
    pos->fiftyMove++;
    
    // clear the captured piece and reset 50 move rule
    if(captured != EMPTY) {
        ASSERT(PieceValid(captured));
        ClearPiece(to, pos);
        pos->fiftyMove = 0;
    }
    
    // increment plys
    pos->hisPly++;
    pos->ply++;
    
    ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXTOTALMOVES);
    ASSERT(pos->ply >= 0 && pos->ply < MAXSEARCHDEPTH);
    
    // set and hash in en passant square
    if(IsPiecePawn[pos->pieces[from]]) {
        pos->fiftyMove = 0;
        if(move & MFLAGPS) {
            if(side==WHITE) {
                pos->enPas=from+10;
                ASSERT(RanksBrd[pos->enPas]==RANK_3);
            } else {
                pos->enPas=from-10;
                ASSERT(RanksBrd[pos->enPas]==RANK_6);
            }
            HASH_EP;
        }
    }
    
    MovePiece(from, to, pos);
    
    // change piece to piece it promotes to
    int prPce = PROMOTED(move);
    if(prPce != EMPTY)   {
        ASSERT(PieceValid(prPce) && !IsPiecePawn[prPce]);
        ClearPiece(to, pos);
        SetPiece(to, pos, prPce);
    }
    
    // update king square
    if(IsPieceKing[pos->pieces[to]]) {
        pos->KingSq[pos->side] = to;
    }
    
    // change and hash side to move
    pos->side ^= 1;
    HASH_SIDE;
    
    ASSERT(BoardListsConsistent(pos));
    
    // if moving into check, take back move and return that it is illegal
    if(IsSqAttacked(pos->KingSq[side],pos->side,pos))  {
        TakeBackMove(pos);
        return FALSE;
    }
    
    return TRUE;
}

// take move back, practically opposite of make move
void TakeBackMove(BoardState *pos) {
    
    ASSERT(BoardListsConsistent(pos));
    
    pos->hisPly--;
    pos->ply--;
    
    ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXTOTALMOVES);
    ASSERT(pos->ply >= 0 && pos->ply < MAXSEARCHDEPTH);
    
    int move = pos->history[pos->hisPly].move;
    int from = FROMSQ(move);
    int to = TOSQ(move);	
    
    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    
    if(pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;
    
    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;
    
    if(pos->enPas != NO_SQ) HASH_EP;
    HASH_CA;
    
    pos->side ^= 1;
    HASH_SIDE;
    
    if(MFLAGEP & move) {
        if(pos->side == WHITE) {
            SetPiece(to-10, pos, bP);
        } else {
            SetPiece(to+10, pos, wP);
        }
    } else if(MFLAGCA & move) {
        switch(to) {
            case C1: MovePiece(D1, A1, pos); break;
            case C8: MovePiece(D8, A8, pos); break;
            case G1: MovePiece(F1, H1, pos); break;
            case G8: MovePiece(F8, H8, pos); break;
            default: ASSERT(FALSE); break;
        }
    }
    
    MovePiece(to, from, pos);
    
    if(IsPieceKing[pos->pieces[from]]) {
        pos->KingSq[pos->side] = from;
    }
    
    int captured = CAPTURED(move);
    if(captured != EMPTY) {
        ASSERT(PieceValid(captured));
        SetPiece(to, pos, captured);
    }
    
    if(PROMOTED(move) != EMPTY)   {
        ASSERT(PieceValid(PROMOTED(move)) && !IsPiecePawn[PROMOTED(move)]);
        ClearPiece(from, pos);
        SetPiece(from, pos, (PieceCol[PROMOTED(move)] == WHITE ? wP : bP));
    }
    
    ASSERT(BoardListsConsistent(pos));
    
}

// https://www.chessprogramming.org/Null_Move_Pruning
// Same code as make move, but not making any move
void MakeNullMove(BoardState *pos) {
    
    ASSERT(BoardListsConsistent(pos));
    ASSERT(!IsSqAttacked(pos->KingSq[pos->side],pos->side^1,pos));
    
    pos->ply++;
    pos->history[pos->hisPly].hashKey = pos->hashKey;
    
    if(pos->enPas != NO_SQ) HASH_EP;
    
    pos->history[pos->hisPly].move = NOMOVE;
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;
    pos->enPas = NO_SQ;
    
    pos->side ^= 1;
    pos->hisPly++;
    HASH_SIDE;
    
    ASSERT(BoardListsConsistent(pos));
    ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXTOTALMOVES);
    ASSERT(pos->ply >= 0 && pos->ply < MAXSEARCHDEPTH);
    
    return;
}

// opposite of MakeNullMove
void TakeBackNullMove(BoardState *pos) {
    ASSERT(BoardListsConsistent(pos));
    
    pos->hisPly--;
    pos->ply--;
    
    if(pos->enPas != NO_SQ) HASH_EP;
    
    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;
    
    if(pos->enPas != NO_SQ) HASH_EP;
    pos->side ^= 1;
    HASH_SIDE;
    
    ASSERT(BoardListsConsistent(pos));
    ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXTOTALMOVES);
    ASSERT(pos->ply >= 0 && pos->ply < MAXSEARCHDEPTH);
}
