// init.c

#include "stdio.h"
#include "stdlib.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

// creates random 64-bit number
/*
 0000 000000000000000 000000000000000 000000000000000 111111111111111
 0000 000000000000000 000000000000000 111111111111111 000000000000000
 0000 000000000000000 111111111111111 000000000000000 000000000000000
 0000 111111111111111 000000000000000 000000000000000 000000000000000
 1111 000000000000000 000000000000000 000000000000000 000000000000000
 */

#define RAND_64 	((U64)rand() | \
(U64)rand() << 15 | \
(U64)rand() << 30 | \
(U64)rand() << 45 | \
((U64)rand() & 0xf) << 60 )

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[BRD_SQ_NUM];
int RanksBrd[BRD_SQ_NUM];

U64 FileBBMask[8];
U64 RankBBMask[8];

U64 BlackPassedBBMask[64];
U64 WhitePassedBBMask[64];
U64 IsolatedBBMask[64];

/* ex: rank bb mask        file bb mask      ( - is 0, x is 1 )
 - - - - - - - -     - - x - - - - -
 x x x x x x x x     - - x - - - - -
 - - - - - - - -     - - x - - - - -
 - - - - - - - -     - - x - - - - -
 - - - - - - - -     - - x - - - - -
 - - - - - - - -     - - x - - - - -
 - - - - - - - -     - - x - - - - -
 - - - - - - - -     - - x - - - - -
 */

/* ex: passed pawn bb mask ex
 - - - 1 1 1 - -
 - - - 1 1 1 - -
 - - - 1 1 1 - -
 - - - 1 1 1 - -
 - - - 1 1 1 - -
 - - - - x - - -
 - - - - - - - -
 - - - - - - - -
 */

/* ex: isolated pawn bb mask ex
 - - - 1 - 1 - -
 - - - 1 - 1 - -
 - - - 1 - 1 - -
 - - - 1 - 1 - -
 - - - 1 - 1 - -
 - - - 1 x 1 - -
 - - - 1 - 1 - -
 - - - 1 - 1 - -
 */

void InitEvalMasks() {
    
    int sq, tsq, r, f;
    
    // reset file and rank bit masks
    for(sq = 0; sq < 8; ++sq) {
        FileBBMask[sq] = 0ULL;
        RankBBMask[sq] = 0ULL;
    }
    
    // set bit masks for each square in corresponding rank and file bit masks
    for(r = RANK_8; r >= RANK_1; r--) {
        for (f = FILE_A; f <= FILE_H; f++) {
            sq = r * 8 + f;
            FileBBMask[f] |= (1ULL << sq);
            RankBBMask[r] |= (1ULL << sq);
        }
    }
    
    // reset isolated and passed pawn bit masks
    for(sq = 0; sq < 64; ++sq) {
        IsolatedBBMask[sq] = 0ULL;
        WhitePassedBBMask[sq] = 0ULL;
        BlackPassedBBMask[sq] = 0ULL;
    }
    
    // set isolated and passed pawn masks for each square
    for(sq = 0; sq < 64; ++sq) {
        tsq = sq + 8;
        
        while(tsq < 64) {
            WhitePassedBBMask[sq] |= (1ULL << tsq);
            tsq += 8;
        }
        
        tsq = sq - 8;
        while(tsq >= 0) {
            BlackPassedBBMask[sq] |= (1ULL << tsq);
            tsq -= 8;
        }
        
        if(FilesBrd[SQ120(sq)] > FILE_A) {
            IsolatedBBMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] - 1];
            
            tsq = sq + 7;
            while(tsq < 64) {
                WhitePassedBBMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }
            
            tsq = sq - 9;
            while(tsq >= 0) {
                BlackPassedBBMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
        
        if(FilesBrd[SQ120(sq)] < FILE_H) {
            IsolatedBBMask[sq] |= FileBBMask[FilesBrd[SQ120(sq)] + 1];
            
            tsq = sq + 9;
            while(tsq < 64) {
                WhitePassedBBMask[sq] |= (1ULL << tsq);
                tsq += 8;
            }
            
            tsq = sq - 7;
            while(tsq >= 0) {
                BlackPassedBBMask[sq] |= (1ULL << tsq);
                tsq -= 8;
            }
        }
    }
}

// get file and rank for each square
void InitFilesRanksBrd() {
    
    int index = 0;
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1;
    
    for(index = 0; index < BRD_SQ_NUM; ++index) {
        FilesBrd[index] = OFFBOARD;
        RanksBrd[index] = OFFBOARD;
    }
    
    for(rank = RANK_1; rank <= RANK_8; ++rank) {
        for(file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file,rank);
            FilesBrd[sq] = file;
            RanksBrd[sq] = rank;
        }
    }
}

// asign random 64-bit numbers to each component of hash key
void InitHashKeys() {
    
    int index = 0;
    int index2 = 0;
    for(index = 0; index < 13; ++index) {
        for(index2 = 0; index2 < 120; ++index2) {
            PieceKeys[index][index2] = RAND_64;
        }
    }
    SideKey = RAND_64;
    for(index = 0; index < 16; ++index) {
        CastleKeys[index] = RAND_64;
    }
    
}

void InitBitMasks() {
    int index = 0;
    
    for(index = 0; index < 64; index++) {
        SetMask[index] = 0ULL;
        ClearMask[index] = 0ULL;
    }
    
    for(index = 0; index < 64; index++) {
        SetMask[index] |= (1ULL << index);
        ClearMask[index] = ~SetMask[index];
    }
}

void InitSq120To64() {
    
    int index = 0;
    int file = FILE_A;
    int rank = RANK_1;
    int sq = A1;
    int sq64 = 0;
    
    // make each square on 120 an off board value
    for(index = 0; index < BRD_SQ_NUM; ++index) {
        Sq120ToSq64[index] = 64;
    }
    
    // make each square on 64 an off board value
    for(index = 0; index < 64; ++index) {
        Sq64ToSq120[index] = 120;
    }
    
    // for each square, correctly translates 120 to 64, and vice versa
    for(rank = RANK_1; rank <= RANK_8; ++rank) {
        for(file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file,rank);
            ASSERT(SqOnBoard(sq));
            Sq64ToSq120[sq64] = sq;
            Sq120ToSq64[sq] = sq64;
            sq64++;
        }
    }
}

void Initialize() {
    InitSq120To64();
    InitBitMasks();
    InitHashKeys();
    InitFilesRanksBrd();
    InitEvalMasks();
    InitMvvLva();
}
