// perft.c

#include "stdio.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

long leafNodes;

// Performance test: https://www.chessprogramming.org/Perft
// goes through entire perft tree
void Perft(int depth, BoardState *pos) {
    
    ASSERT(BoardListsConsistent(pos));
    
    // incrememnt leafNodes if end of branch
    if(depth == 0) {
        leafNodes++;
        return;
    }	
    
    MoveList list[1];
    GenerateAllMoves(pos,list);
    
    // loop through all moves in movelist
    int MoveNum = 0;
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {	
        
        // ignore illegal moves
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        
        // recursively call Perft w one less depth and take back move
        Perft(depth - 1, pos);
        TakeBackMove(pos);
    }
    return;
}

// perform performance test
void PerftTest(int depth, BoardState *pos) {
    
    ASSERT(BoardListsConsistent(pos));
    
    PrintBoard(pos);
    printf("\nStarting Test To Depth:%d\n",depth);	
    leafNodes = 0;
    int start = GetTimeMs();
    MoveList list[1];
    GenerateAllMoves(pos,list);	
    
    int move;	    
    int MoveNum = 0;
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        move = list->moves[MoveNum].move;
        if ( !MakeMove(pos,move))  {
            continue;
        }
        long cumnodes = leafNodes;
        Perft(depth - 1, pos);
        TakeBackMove(pos);        
        long oldnodes = leafNodes - cumnodes;
        printf("move %d : %s : %ld\n",MoveNum+1,PrMove(move),oldnodes);
    }
    
    printf("\nTest Complete : %ld nodes visited in %dms\n",leafNodes,GetTimeMs() - start);
    
    return;
}
