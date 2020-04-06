#ifndef DEFS_H
#define DEFS_H

#include "stdlib.h"
#include "stdio.h"

// MARK: DEFINITIONS

// MARK: - 1. Debugging Definition

// #define DEBUG    // <-- uncomment to turn on debug mode

#define MAX_HASH 1024

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

// MARK: - 2. Type Definiteions

typedef unsigned long long U64;
#define BOOL int

// MARK: - 3. Program Constants

#define NAME "SECS 1.0"
#define BRD_SQ_NUM 120
/* 10x12 board configuration used for edge detection
 x  x  x  x  x  x  x  x  x  x
 x  x  x  x  x  x  x  x  x  x
 x  r  n  b  q  k  b  n  r  x
 x  p  p  p  p  p  p  p  p  x
 x  .  .  .  .  .  .  .  .  x
 x  .  .  .  .  .  .  .  .  x
 x  .  .  .  .  .  .  .  .  x
 x  .  .  .  .  .  .  .  .  x
 x  P  P  P  P  P  P  P  P  x
 x  R  N  B  Q  K  B  N  R  x
 x  x  x  x  x  x  x  x  x  x
 x  x  x  x  x  x  x  x  x  x
 */

#define MAXTOTALMOVES 1024   // 512 full moves which is plenty (https://www.chess.com/article/view/the-4-longest-chess-games-in-history)
#define MAXPOSITIONMOVES 256    // theoretical max is 218 (https://chess.stackexchange.com/questions/4490/maximum-possible-movement-in-a-turn)
#define MAXSEARCHDEPTH 64

#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define INFINITE 30000  // close enough, used for mate score
#define ISMATE (INFINITE - MAXSEARCHDEPTH)

// MARK: - 4. Program Enumerations (from VICE)

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK  };
enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE };
enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE };

enum { WHITE, BLACK, BOTH };
enum { UCIMODE, XBOARDMODE, CONSOLEMODE };
enum {
    A1 = 21, B1, C1, D1, E1, F1, G1, H1,
    A2 = 31, B2, C2, D2, E2, F2, G2, H2,
    A3 = 41, B3, C3, D3, E3, F3, G3, H3,
    A4 = 51, B4, C4, D4, E4, F4, G4, H4,
    A5 = 61, B5, C5, D5, E5, F5, G5, H5,
    A6 = 71, B6, C6, D6, E6, F6, G6, H6,
    A7 = 81, B7, C7, D7, E7, F7, G7, H7,
    A8 = 91, B8, C8, D8, E8, F8, G8, H8, NO_SQ, OFFBOARD  // no_sq used for en passant
};

enum { FALSE, TRUE };

// castling rights (0 0 0 0). ex: 1 0 0 0 indicates black queenside castle still legal, none other
enum { WKCA = 1, WQCA = 2, BKCA = 4, BQCA = 8 };

// MARK: - 5. Program Structure Types

// all move info: from square, to square, what piece is captured (if any), if en passant capture, if pawn start, what piece promoting to (if any), if a castling move is stored along with the move's evaluation
typedef struct {
    int move;
    int score;
} Move;

// list of moves stored. Array of moves and an int giving count of moves in move list
typedef struct {
    Move moves[MAXPOSITIONMOVES];
    int count;
} MoveList;

// HFEXACT = beat alpha but not beta
enum {  HFNONE, HFALPHA, HFBETA, HFEXACT};

// used to probe transposition (hash table) for previously searched moves or for principal variation.
typedef struct {
    U64 hashKey;
    int move;
    int score;
    int depth;
    int flags;
} HashEntry;

typedef struct {
    HashEntry *hashEntry;
    int numEntries;
    int newWrite;
    int overWrite;
    int hit;    // count of how many times hash table is actually used
    int cut;
} HashTable;

typedef struct {
    int move;
    int castlePerm;
    int enPas;
    int fiftyMove;
    U64 hashKey;
} Undo;

typedef struct {
    int pieces[BRD_SQ_NUM];
    U64 pawns[3];   // white pawns bitboard, black pawns bitboard, all pawns bitboard (https://www.chessprogramming.org/Bitboards)
    
    int KingSq[2];  // white king square, white king square, used for easy check detection
    
    int side;   // side to move
    int enPas;  // en passant square position
    int fiftyMove;  // number of moves since last pawn move or capture for 50 move rule detection
    
    int ply;    // half-moves into current search
    int hisPly; // half-moves into game (used for repetition detection)
    
    int castlePerm; // 0-8 storing castle permissions
    
    U64 hashKey; // unique key for each position
    
    int pceNum[13]; // number of pieces on board for each piece type
    int bigPce[2];  // number of big pieces (not pawns) by color
    int majPce[2];  // number of major pieces by color
    int minPce[2];  // number of minor pieces by color
    int material[2];    // material score for each side
    
    Undo history[MAXTOTALMOVES];   // allow undo to previous game positions
    
    // piece list
    int pieceList[13][10];  // [piece number in array of pieces][max number of pieces on the board (all pawns promoted)], returns position of piece (https://www.chessprogramming.org/Piece-Lists)
    
    // hash table for storing searched moves and PV, PVArray for storing exact PV
    HashTable hashTable[1];
    int PVArray[MAXSEARCHDEPTH];
    
    // if move beats alpha, the piece and its to square in searchHistory incremented by 1 (https://www.chessprogramming.org/History_Heuristic)
    int searchHistory[13][BRD_SQ_NUM];
    // stores 2 most recent beta cutoffs (https://www.chessprogramming.org/Killer_Heuristic)
    int searchKillers[2][MAXSEARCHDEPTH];
} BoardState;

typedef struct {
    int starttime;
    int stoptime;
    int depth;
    int timeset;    // UCI can limit time for search, set here.
    int movestogo;  // moves until time is renewed
    
    long nodes; // total number of nodes visited in search
    
    int quit;       // GUI can tell to quit
    int stopped;    // GUI can tell to stop
    
    // indicators to see how good search ordering is
    float fh;       // fail high (https://www.chessprogramming.org/Fail-High)
    float fhf;      // fail high first
    int nullCut;
    
    int GAME_MODE;      // XBoard or UCI or terminal
    int POST_THINKING;  // option to not show engine thinking
    
} Search;

#endif
