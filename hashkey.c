// hashkeys.c
#include "stdio.h"
#include "defs.h"

U64 GeneratePosKey(const S_BOARD *pos) {

    int sq = 0;
    U64 finalKey = 0;
    int piece = EMPTY;
    
    // pieces hash
    for(sq = 0; sq < BRD_SQ_NUM; ++sq) {
        piece = pos->pieces[sq];
        if(piece!=NO_SQ && piece!=EMPTY && piece!=OFFBOARD) {
            ASSERT(piece>=wP && piece<=bK);
            finalKey ^= PieceKeys[piece][sq];
        }
    }
    
    // side to move hash
    if(pos->side == WHITE) {
        finalKey ^= SideKey;
    }
        
    // en passant hash
    if(pos->enPas != NO_SQ) {
        ASSERT(pos->enPas>=0 && pos->enPas<BRD_SQ_NUM);
        finalKey ^= PieceKeys[EMPTY][pos->enPas];   // not creating en passant hash because already created random values in piece keys, so just setting to whatever empty square hash value is on the en passant square and hashing that in.
    }
    
    ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15);
    
    // castling permissions hash
    finalKey ^= CastleKeys[pos->castlePerm];
    
    return finalKey;
}


