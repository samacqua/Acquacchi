#include "stdio.h"
#include "defs.h"

// https://www.chessprogramming.org/BitScan#Matt_Taylor.27s_Folding_trick
const int MagicBitTable[64] = {
  63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
  51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
  26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
  58, 20, 37, 17, 36, 8
};

int PopBit(U64 *bb) {
  U64 b = *bb ^ (*bb - 1);
  unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return MagicBitTable[(fold * 0x783a9b23) >> 26];
}

int CountBits(U64 b) {
  int r;
  for(r = 0; b; r++, b &= b - 1);
  return r;
}

void PrintBitBoard(U64 bb) {

    U64 shifter = 1ULL;
    
    int rank = 0;
    int file = 0;
    int sq = 0;
    int sq64 = 0;
    
    printf("\n");
    for(rank = RANK_8; rank >= RANK_1; --rank) {
        for(file = FILE_A; file <= FILE_H; ++file) {
            sq = FR2SQ(file,rank);    // 120 based
            sq64 = SQ64(sq); // 64 based
            
            if((shifter << sq64) & bb)
                printf("%s", "🔴");
            else
                printf("🔵");
                
        }
        printf("\n");
    }
    printf("\n\n");
}

const int KnDir[8] = { 6, 15, 17, 10, -6, -15, -17, -10 };
const int RkDir[4] = { -1, -10,    1, 10 };
const int BiDir[4] = { -9, -11, 11, 9 };
const int KiDirWe[3] = { -1, -9, 7 };
const int KiDirCe[3] = {-8, 8 };
const int KiDirEa[3] = { -7, 1, 9 };

const U64 notAFile = 0xfefefefefefefefe; // ~0x0101010101010101
const U64 notABFile = 0xfcfcfcfcfcfcfcfc;
const U64 notGHFile = 0x3f3f3f3f3f3f3f3f;
const U64 notHFile = 0x7f7f7f7f7f7f7f7f; // ~0x8080808080808080
const U64 notBorder = 0x007e7e7e7e7e7e00;

const U64 Rank1BB = 0x00000000000000ff;
const U64 not1Rank = 0xffffffffffffff00;
const U64 not8Rank = 0x00ffffffffffffff;

U64 Shift(U64 bb, int direction) {
    return direction > 0 ? bb << direction: bb >> -direction;
}

// https://www.chessprogramming.org/General_Setwise_Operations#One_Step_Only
U64 PawnAttacksBB(U64 bb, int color) {
    return color == WHITE ? (Shift(bb, NORTHWEST)&notHFile) | (Shift(bb, NORTHEAST)&notAFile) : Shift(bb, SOUTHWEST) | Shift(bb, SOUTHEAST);
}

U64 KnightAttacksBB(U64 bb) {
    return Shift(bb, NORTH+NORTH+EAST)&notAFile \
        | Shift(bb, NORTH+NORTH+WEST)&notHFile \
        | Shift(bb, EAST+EAST+NORTH)&notABFile \
        | Shift(bb, EAST+EAST+SOUTH)&notABFile \
        | Shift(bb, SOUTH+SOUTH+EAST)&notAFile \
        | Shift(bb, SOUTH+SOUTH+WEST)&notHFile \
        | Shift(bb, WEST+WEST+NORTH)&notGHFile \
        | Shift(bb, WEST+WEST+SOUTH)&notGHFile;
}



U64 KingAttacksBB(U64 bb) {
    return Shift(bb, NORTH) | Shift(bb, NORTHEAST)&notAFile | Shift(bb, EAST)&notAFile | Shift(bb, SOUTHEAST)&notAFile | Shift(bb, SOUTH) | Shift(bb, SOUTHWEST)&notHFile | Shift(bb, WEST)&notHFile | Shift(bb, NORTHWEST)&notHFile;
}

U64 SoutAttacks(U64 rooks, U64 empty) {
   U64 flood = rooks;
   flood |= rooks = (rooks >> 8) & empty;
   flood |= rooks = (rooks >> 8) & empty;
   flood |= rooks = (rooks >> 8) & empty;
   flood |= rooks = (rooks >> 8) & empty;
   flood |= rooks = (rooks >> 8) & empty;
   flood |=         (rooks >> 8) & empty;
   return            flood >> 8;
}

U64 NortAttacks(U64 rooks, U64 empty) {
   U64 flood = rooks;
   flood |= rooks = (rooks << 8) & empty;
   flood |= rooks = (rooks << 8) & empty;
   flood |= rooks = (rooks << 8) & empty;
   flood |= rooks = (rooks << 8) & empty;
   flood |= rooks = (rooks << 8) & empty;
   flood |=         (rooks << 8) & empty;
   return            flood << 8;
}

U64 EastAttacks(U64 rooks, U64 empty) {
   U64 flood = rooks;
   empty &= notAFile;
   flood |= rooks = (rooks << 1) & empty;
   flood |= rooks = (rooks << 1) & empty;
   flood |= rooks = (rooks << 1) & empty;
   flood |= rooks = (rooks << 1) & empty;
   flood |= rooks = (rooks << 1) & empty;
   flood |=         (rooks << 1) & empty;
   return           (flood << 1) & notAFile ;
}

U64 NoEaAttacks(U64 bishops, U64 empty) {
   U64 flood = bishops;
   empty &= notAFile;
   flood |= bishops = (bishops << 9) & empty;
   flood |= bishops = (bishops << 9) & empty;
   flood |= bishops = (bishops << 9) & empty;
   flood |= bishops = (bishops << 9) & empty;
   flood |= bishops = (bishops << 9) & empty;
   flood |=           (bishops << 9) & empty;
   return               (flood << 9) & notAFile ;
}

U64 SoEaAttacks(U64 bishops, U64 empty) {
   U64 flood = bishops;
   empty &= notAFile;
   flood |= bishops = (bishops >> 7) & empty;
   flood |= bishops = (bishops >> 7) & empty;
   flood |= bishops = (bishops >> 7) & empty;
   flood |= bishops = (bishops >> 7) & empty;
   flood |= bishops = (bishops >> 7) & empty;
   flood |=           (bishops >> 7) & empty;
   return               (flood >> 7) & notAFile ;
}

U64 WestAttacks(U64 rooks, U64 empty) {
   U64 flood = rooks;
   empty &= notHFile;
   flood |= rooks = (rooks >> 1) & empty;
   flood |= rooks = (rooks >> 1) & empty;
   flood |= rooks = (rooks >> 1) & empty;
   flood |= rooks = (rooks >> 1) & empty;
   flood |= rooks = (rooks >> 1) & empty;
   flood |=         (rooks >> 1) & empty;
   return           (flood >> 1) & notHFile ;
}

U64 SoWeAttacks(U64 bishops, U64 empty) {
   U64 flood = bishops;
   empty &= notHFile;
   flood |= bishops = (bishops >> 9) & empty;
   flood |= bishops = (bishops >> 9) & empty;
   flood |= bishops = (bishops >> 9) & empty;
   flood |= bishops = (bishops >> 9) & empty;
   flood |= bishops = (bishops >> 9) & empty;
   flood |=           (bishops >> 9) & empty;
   return               (flood >> 9) & notHFile ;
}

U64 NoWeAttacks(U64 bishops, U64 empty) {
   U64 flood = bishops;
   empty &= notHFile;
   flood |= bishops = (bishops << 7) & empty;
   flood |= bishops = (bishops << 7) & empty;
   flood |= bishops = (bishops << 7) & empty;
   flood |= bishops = (bishops << 7) & empty;
   flood |= bishops = (bishops << 7) & empty;
   flood |=           (bishops << 7) & empty;
   return               (flood << 7) & notHFile ;
}

U64 BishopAttacksBB(U64 bishopsbb, U64 emptybb) {
    return (NoEaAttacks(bishopsbb, emptybb) | SoEaAttacks(bishopsbb, emptybb) | SoWeAttacks(bishopsbb, emptybb) | NoWeAttacks(bishopsbb, emptybb)) & notBorder;
}

U64 RooksAttackBB(U64 rooksbb, U64 emptybb) {
    const U64 notBottomTop = 0x00ffffffffffff00;
    const U64 notAHFile = notAFile & notHFile;
    return (NortAttacks(rooksbb, emptybb) | SoutAttacks(rooksbb, emptybb))&notBottomTop | (EastAttacks(rooksbb, emptybb) | WestAttacks(rooksbb, emptybb))&notAHFile;
}

U64 QueenAttackBB(U64 queensbb, U64 emptybb) {
    return RooksAttackBB(queensbb, emptybb) | BishopAttacksBB(queensbb, emptybb);
}

int RankOf(int s) {
    return Shift(s, -3);
}

U64 rank_bb(int square) {
    return Rank1BB << (8 * RankOf(square));
}

int FileOf(int s) {
    return s & 7;
}

U64 file_bb(int square) {
    return ~notAFile << FileOf(square);
}

U64 SlidingAttacks(int directions[], int square, U64 occupied) {
    U64 attack = 0ULL;
    for (int i = 0; i < 4; ++i) {
        for (int s = square + directions[i]; s += directions[i];) {
            attack |= s;

            if (occupied & s) {
                break;
            }
        }
    }

    return attack;
}

// from stockfish
void InitMagics(MAGIC magics[]) {
    
    U64 Rank8BB = 0xff00000000000000;
    
    U64 occupancy[4096], reference[4096], edges, b;
    int epoch[4096] = {}, cnt = 0, size = 0;
    
    // Optimal PRNG seeds to pick the correct magics in the shortest time
    int seeds[][8] = { { 8977, 44560, 54343, 38998,  5731, 95205, 104912, 17020 },
    {  728, 10316, 55013, 32803, 12281, 15100,  16645,   255 } };
    
    for (int square = 0; square < 64; ++square) {
        
        // Board edges are not considered in the relevant occupancies
        edges = ((Rank1BB | Rank8BB) & ~rank_bb(square)) | ((~notAFile | ~notHFile) & ~file_bb(square));
        PrintBitBoard(edges);

        MAGIC* m = &magics[square];
        m->mask = 0ULL;
        m->shift = 0;
    }
}



void InitMoveBitBoards() {
    int square = 63;
//    printf("%d", RankOf(square));
//    PrintBitBoard(rank_bb(32));
    
    for (int square = 0; square < 64; ++square) {
        U64 occupied = 0ULL;
        int directions[] = { NORTH, EAST, SOUTH, WEST };
        PrintBitBoard(SlidingAttacks(directions, 36, occupied));
    }
    
//    InitMagics();
}

