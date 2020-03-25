#include "stdio.h"
#include "defs.h"

#define FEN1 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
#define FEN2 "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"
#define FEN3 "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2"

int main() {

    AllInit();
    
//    S_BOARD board[1];
//    
//    ParseFen(START_FEN, board);
//    PrintBoard(board);
//    
//    ParseFen(FEN1, board);
//    PrintBoard(board);
//    
//    ParseFen(FEN2, board);
//    PrintBoard(board);
//    
//    ParseFen(FEN3, board);
//    PrintBoard(board);
//    
//    ASSERT(CheckBoard(board));
//    
//    SqAttacked(23, 0);
    
    /*  check pop and count of bit boards, check setting bit boards
    
    U64 pawnBB = 0ULL;
    U64 knightBB = 0ULL;
    U64 bishopBB = 0ULL;
    U64 rookBB = 0ULL;
    U64 queenBB = 0ULL;
    U64 kingBB = 0ULL;
    
    pawnBB |= (1ULL << SQ64(D2));
    pawnBB |= (1ULL << SQ64(D3));
    pawnBB |= (1ULL << SQ64(D4));
    
    int sq64 = 0;
    
    while (pawnBB) {
        sq64 = PopBit(&pawnBB);
        printf("popped:%d\n",sq64);
        PrintBitBoard(pawnBB, PAWN_BB);
    }
     
     */
        
    return 0;
}

