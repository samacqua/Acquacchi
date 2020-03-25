// init.c

#include "defs.h"

// rand() creates 15 bit random number, so make it 64-bit
#define RAND_64 (    (U64)rand() | \
(U64)rand() << 15 | \
(U64)rand() << 30 | \
(U64)rand() << 45 | \
((U64)rand() & 0xf) << 60    )

int Sq120ToSq64[BRD_SQ_NUM];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

// hash keys
U64 PieceKeys[13][120]; // will be random number for each square for each piece used for position hashing
U64 SideKey;
U64 CastleKeys[16]; // 0 0 0 0

// fills each hash key with a random number
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
	for(index = 0; index < BRD_SQ_NUM; ++index) {
		Sq120ToSq64[index] = 65;
	}
	
	for(index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120;
	}
	
	for(rank = RANK_1; rank <= RANK_8; ++rank) {
		for(file = FILE_A; file <= FILE_H; ++file) {
			sq = FR2SQ(file,rank);
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq] = sq64;
			sq64++;
		}
	}
}

void AllInit() {
	InitSq120To64();
    InitBitMasks();
    InitHashKeys();
    InitMoveBitBoards();
}
