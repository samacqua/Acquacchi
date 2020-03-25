#ifndef DEFS_H
#define DEFS_H

#include "stdlib.h"

#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

typedef unsigned long long U64;

#define NAME "SECS 1.0"
#define BRD_SQ_NUM 120

#define MAXGAMEMOVES 2048

#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };
enum { WHITE_BB, BLACK_BB, PAWN_BB, KNIGHT_BB, BISHOP_BB, ROOK_BB, QUEEN_BB, KING_BB };
enum { WHITE, BLACK, BOTH };

enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD
};

enum { FALSE, TRUE };

enum { NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1, NORTHEAST = 9, NORTHWEST = 7, SOUTHEAST = -7, SOUTHWEST = -9 };

enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };

typedef struct {
    
    int pieces[BRD_SQ_NUM]; // piece on each square of board
    
    U64 bitboards[8];   // white, black, pawn, knight, bishop, rook, queen, king
    
    int material[2];
    
    int side;
    int enPas;
    int fiftyMove;
    
    int searchPly;
    int movePly;
    
    int castlePerm;
    
    U64 posKey;
    
} S_BOARD;

typedef struct {
    
    int move;
    int castlePerm;
    int enPas;
    int fiftyMove;
    U64 posKey;

} S_UNDO;

// Magic holds all magic bitboards relevant data for a single square
typedef struct {
    U64 mask;
    U64 magic;
    U64* attacks;
    int shift;
} MAGIC;

/* MACROS */

#define FR2SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) )   // converts file and rank to 120 square
#define SQ64(sq120) Sq120ToSq64[sq120]
#define SQ120(sq64) (Sq64ToSq120[(sq64)])
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])

/* GLOBALS */

// board formatting
extern int Sq120ToSq64[BRD_SQ_NUM];
extern int Sq64ToSq120[64];

// bitwise operations
extern U64 SetMask[64];
extern U64 ClearMask[64];

// hash keys
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];

// printing board to terminal
extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

// piece groups
extern int PieceVal[13];
extern int PieceCol[13];

/* FUNCTIONS */

// init.c
extern void AllInit();

// bitboards.c
extern void PrintBitBoard(U64 bb);
extern int PopBit(U64 *bb);
extern int CountBits(U64 b);
void InitMoveBitBoards();

// hashkeys.c
extern U64 GeneratePosKey(const S_BOARD *pos);

// board.c
extern void ResetBoard(S_BOARD *pos);
extern int ParseFen(char *fen, S_BOARD *pos);
extern void PrintBoard(const S_BOARD *pos);
extern void UpdateBitBoards(S_BOARD *pos);
extern int CheckBoard(const S_BOARD *pos);

// attack.c
extern int SqAttacked(const int sq, const int side);

#endif

