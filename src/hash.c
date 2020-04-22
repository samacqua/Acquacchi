// hash.c
#include "stdio.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

// generate hash key using Zobrist Hashing (https://www.chessprogramming.org/Zobrist_Hashing)
U64 GenerateHashKey(const BoardState *pos) {
    
    int sq = 0;
    U64 finalKey = 0;
    int piece = EMPTY;
    
    // pieces
    for(sq = 0; sq < BRD_SQ_NUM; ++sq) {
        piece = pos->pieces[sq];
        if(piece!=NO_SQ && piece!=EMPTY && piece != OFFBOARD) {
            ASSERT(piece>=wP && piece<=bK);
            finalKey ^= PieceKeys[piece][sq];
        }		
    }
    
    // side to move
    if(pos->side == WHITE) {
        finalKey ^= SideKey;
    }
    
    // en passant square
    if(pos->enPas != NO_SQ) {
        ASSERT(pos->enPas>=0 && pos->enPas<BRD_SQ_NUM);
        ASSERT(SqOnBoard(pos->enPas));
        ASSERT(RanksBrd[pos->enPas] == RANK_3 || RanksBrd[pos->enPas] == RANK_6);
        finalKey ^= PieceKeys[EMPTY][pos->enPas];
    }
    
    ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15);
    
    // castle permissions
    finalKey ^= CastleKeys[pos->castlePerm];
    
    return finalKey;
}

// get PV move from hash table
int ProbePVMove(const BoardState *pos) {
    
    int index = pos->hashKey % pos->hashTable->numEntries;
    ASSERT(index >= 0 && index <= pos->hashTable->numEntries - 1);
    
    if( pos->hashTable->hashEntry[index].hashKey == pos->hashKey ) {
        return pos->hashTable->hashEntry[index].move;
    }
    
    return NOMOVE;
}

int GetPVLine(const int depth, BoardState *pos) {
    
    ASSERT(depth < MAXSEARCHDEPTH && depth >= 1);
    
    int move = ProbePVMove(pos);
    int count = 0;
    
    // loop while there is a move and still within depth of search
    while(move != NOMOVE && count < depth) {
        
        ASSERT(count < MAXSEARCHDEPTH);
        
        // add move to PVArray and increment index
        if( MoveExists(pos, move) ) {
            MakeMove(pos, move);
            pos->PVArray[count++] = move;
        } else {
            break;
        }
        move = ProbePVMove(pos);
    }
    
    // go back to root so can continue searching
    while(pos->ply > 0) {
        TakeBackMove(pos);
    }
    
    return count;
    
}

void ClearHashTable(HashTable *table) {
    
    HashEntry *tableEntry;
    
    // reset each entry
    for (tableEntry = table->hashEntry; tableEntry < table->hashEntry + table->numEntries; tableEntry++) {
        tableEntry->hashKey = 0ULL;
        tableEntry->move = NOMOVE;
        tableEntry->depth = 0;
        tableEntry->score = 0;
        tableEntry->flags = 0;
    }
    table->newWrite=0;
}

// get a move from the hash table
// from https://web.archive.org/web/20071031100051/http://www.brucemo.com/compchess/programming/hashing.htm
int ProbeHashEntry(BoardState *pos, int *move, int *score, int alpha, int beta, int depth) {
    
    int index = pos->hashKey % pos->hashTable->numEntries;
    
    ASSERT(index >= 0 && index <= pos->hashTable->numEntries - 1);
    ASSERT(depth>=1&&depth<MAXSEARCHDEPTH);
    ASSERT(alpha<beta);
    ASSERT(alpha>=-INFINITE&&alpha<=INFINITE);
    ASSERT(beta>=-INFINITE&&beta<=INFINITE);
    ASSERT(pos->ply>=0&&pos->ply<MAXSEARCHDEPTH);
    
    // if position is stored in hash table and is at least the depth we are searching
    if( pos->hashTable->hashEntry[index].hashKey == pos->hashKey ) {
        *move = pos->hashTable->hashEntry[index].move;
        if(pos->hashTable->hashEntry[index].depth >= depth){
            
            pos->hashTable->hit++;
            
            ASSERT(pos->hashTable->hashEntry[index].depth>=1&&pos->hashTable->hashEntry[index].depth<MAXSEARCHDEPTH);
            ASSERT(pos->hashTable->hashEntry[index].flags>=HFALPHA&&pos->hashTable->hashEntry[index].flags<=HFEXACT);
            
            *score = pos->hashTable->hashEntry[index].score;
            if(*score > ISMATE) *score -= pos->ply;
            else if(*score < -ISMATE) *score += pos->ply;
            
            switch(pos->hashTable->hashEntry[index].flags) {
                    
                    ASSERT(*score>=-INFINITE&&*score<=INFINITE);
                    
                case HFALPHA: if(*score<=alpha) {
                    *score=alpha;
                    return TRUE;
                }
                    break;
                case HFBETA: if(*score>=beta) {
                    *score=beta;
                    return TRUE;
                }
                    break;
                case HFEXACT:
                    return TRUE;
                    break;
                default: ASSERT(FALSE); break;
            }
        }
    }
    return FALSE;
}

// store a move in hash table
void StoreHashEntry(BoardState *pos, const int move, int score, const int flags, const int depth) {
    
    int index = pos->hashKey % pos->hashTable->numEntries;
    
    ASSERT(index >= 0 && index <= pos->hashTable->numEntries - 1);
    ASSERT(depth>=1&&depth<MAXSEARCHDEPTH);
    ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
    ASSERT(score>=-INFINITE&&score<=INFINITE);
    ASSERT(pos->ply>=0&&pos->ply<MAXSEARCHDEPTH);
    
    if( pos->hashTable->hashEntry[index].hashKey == 0) {
        pos->hashTable->newWrite++;
    } else {
        pos->hashTable->overWrite++;
    }
    
    if(score > ISMATE) score += pos->ply;
    else if(score < -ISMATE) score -= pos->ply;
    
    pos->hashTable->hashEntry[index].move = move;
    pos->hashTable->hashEntry[index].hashKey = pos->hashKey;
    pos->hashTable->hashEntry[index].flags = flags;
    pos->hashTable->hashEntry[index].score = score;
    pos->hashTable->hashEntry[index].depth = depth;
}

