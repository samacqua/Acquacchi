// search.c

#include "stdio.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

int rootDepth;

// check if GUI interrupts or if no time left
static void CheckSearchStatus(Search *info) {
    if(info->timeset == TRUE && GetTimeMs() > info->stoptime) {
        info->stopped = TRUE;
    }
    ParseGUIInput(info);
}

static void ChooseBestMove(int moveNum, MoveList *list) {
    
    Move temp;
    int index = 0;
    int bestScore = 0;
    int bestNum = moveNum;
    
    // loop through moves list and find best move
    for (index = moveNum; index < list->count; ++index) {
        if (list->moves[index].score > bestScore) {
            bestScore = list->moves[index].score;
            bestNum = index;
        }
    }
    
    ASSERT(moveNum>=0 && moveNum<list->count);
    ASSERT(bestNum>=0 && bestNum<list->count);
    ASSERT(bestNum>=moveNum);
    
    // swap the current move and best move so that the move that is searched is the best move
    temp = list->moves[moveNum];
    list->moves[moveNum] = list->moves[bestNum];
    list->moves[bestNum] = temp;
}

// check if position is a repitition
static int IsRepetition(const BoardState *pos) {
    
    int index = 0;
    
    // looping through moves in history, starting at last time fifty move rule reset because reset when capture or pawn move, which cannot be repeated
    for(index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index) {
        ASSERT(index >= 0 && index < MAXTOTALMOVES);
        // if current position is equal to past position, it is a repitition
        if(pos->hashKey == pos->history[index].hashKey) {
            return TRUE;
        }
    }
    return FALSE;
}

static void ClearForSearch(BoardState *pos, Search *info) {
    
    int index = 0;
    int index2 = 0;
    
    // reset history heuristic
    for(index = 0; index < 13; ++index) {
        for(index2 = 0; index2 < BRD_SQ_NUM; ++index2) {
            pos->searchHistory[index][index2] = 0;
        }
    }
    
    // reset killer heuristic
    for(index = 0; index < 2; ++index) {
        for(index2 = 0; index2 < MAXSEARCHDEPTH; ++index2) {
            pos->searchKillers[index][index2] = 0;
        }
    }
    
    // reset hash table stats, not the hash table itself
    pos->hashTable->overWrite=0;
    pos->hashTable->hit=0;
    pos->hashTable->cut=0;
    pos->ply = 0;
    
    // reset search info
    info->stopped = 0;
    info->nodes = 0;
    info->fh = 0;
    info->fhf = 0;
}

// necessary to avoid horizon effect (https://www.chessprogramming.org/Quiescence_Search)
static int Quiescence(int alpha, int beta, BoardState *pos, Search *info) {
    
    ASSERT(BoardListsConsistent(pos));
    ASSERT(beta>alpha);
    if(( info->nodes & 2047 ) == 0) {
        CheckSearchStatus(info);
    }
    
    info->nodes++;
    
    // check for 50 move repetion
    if(IsRepetition(pos) || pos->fiftyMove >= 100) {
        return 0;
    }
    
    // if search done, return evaluation
    if(pos->ply > MAXSEARCHDEPTH - 1) {
        return PositionEvaluation(pos);
    }
    
    int Score = PositionEvaluation(pos);
    
    ASSERT(Score>-INFINITE && Score<INFINITE);
    
    // beta cutoff
    if(Score >= beta) {
        return beta;
    }
    
    // alpha cutoff
    if(Score > alpha) {
        alpha = Score;
    }
    
    // same as alpha beta, but calling Quiescence recursively instead of calling Alpha-Beta, and taking out updating of killers and history
    MoveList list[1];
    GenerateAllCaptureMoves(pos,list);
    
    int MoveNum = 0;
    int Legal = 0;
    Score = -INFINITE;
    
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        
        ChooseBestMove(MoveNum, list);
        
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        
        Legal++;
        Score = -Quiescence( -beta, -alpha, pos, info);
        TakeBackMove(pos);
        
        if(info->stopped == TRUE) {
            return 0;
        }
        
        if(Score > alpha) {
            if(Score >= beta) {
                if(Legal==1) {
                    info->fhf++;
                }
                info->fh++;
                return beta;
            }
            alpha = Score;
        }
    }
    
    ASSERT(alpha >= OldAlpha);
    
    return alpha;
}

// Alpha-Beta Pruning https://en.wikipedia.org/wiki/Alpha%E2%80%93beta_pruning
static int AlphaBeta(int alpha, int beta, int depth, BoardState *pos, Search *info, int DoNull) {
    
    ASSERT(BoardListsConsistent(pos));
    ASSERT(beta>alpha);
    ASSERT(depth>=0);
    
    // call Quiescence if reached end of search to ensure no horizon effect
    if(depth <= 0) {
        return Quiescence(alpha, beta, pos, info);
    }
    
    // every once in a while check up on time and on update from GUI
    if(( info->nodes & 2047 ) == 0) {
        CheckSearchStatus(info);
    }
    
    info->nodes++;
    
    // check if 50 move repetition
    if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
        return 0;
    }
    
    // at max depth, return evaluation
    if(pos->ply > MAXSEARCHDEPTH - 1) {
        return PositionEvaluation(pos);
    }
    
    // if in check, continue searching until out of check
    int InCheck = IsSqAttacked(pos->KingSq[pos->side],pos->side^1,pos);
    if(InCheck == TRUE) {
        depth++;
    }
    
    int Score = -INFINITE;
    int PVMove = NOMOVE;
    
    // if move already exists in hash table, return its score
    if( ProbeHashEntry(pos, &PVMove, &Score, alpha, beta, depth) == TRUE ) {
        pos->hashTable->cut++;
        return Score;
    }
    
    // null move heuristic: https://www.chessprogramming.org/Null_Move_Pruning
    if( DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 0) && depth >= 4) {
        MakeNullMove(pos);
        Score = -AlphaBeta( -beta, -beta + 1, depth-4, pos, info, FALSE);   // makeNullMove == FALSE so not just continually flipping sides
        TakeBackNullMove(pos);
        if(info->stopped == TRUE) {
            return 0;
        }
        
        if (Score >= beta && abs(Score) < ISMATE) {
            info->nullCut++;
            return beta;
        }
    }
    
    MoveList list[1];
    GenerateAllMoves(pos,list);
    
    int MoveNum = 0;
    int Legal = 0;
    int OldAlpha = alpha;
    int BestMove = NOMOVE;
    
    int BestScore = -INFINITE;
    
    Score = -INFINITE;
    
    // check if PVMove
    if( PVMove != NOMOVE) {
        for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
            if( list->moves[MoveNum].move == PVMove) {
                list->moves[MoveNum].score = 2000000;
                break;
            }
        }
    }
    
    // loop through move list
    for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
        
        ChooseBestMove(MoveNum, list);
        
        // illegal move, continue
        if ( !MakeMove(pos,list->moves[MoveNum].move))  {
            continue;
        }
        
        Legal++;
        Score = -AlphaBeta( -beta, -alpha, depth-1, pos, info, TRUE);
        TakeBackMove(pos);
        
        if(info->stopped == TRUE) {
            return 0;
        }
        
        // can improve score
        if(Score > BestScore) {
            BestScore = Score;
            BestMove = list->moves[MoveNum].move;
            // alpha cutoff
            if(Score > alpha) {
                // beta cutoff
                if(Score >= beta) {
                    if(Legal==1) {
                        info->fhf++;
                    }
                    info->fh++;
                    
                    // add to killer heuristic
                    if(!(list->moves[MoveNum].move & MFLAGCAP)) {
                        pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
                        pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
                    }
                    // store w/ beta tag and return beta
                    StoreHashEntry(pos, BestMove, beta, HFBETA, depth);
                    return beta;
                }
                // update alpha
                alpha = Score;
                // add to history heuristic
                if(!(list->moves[MoveNum].move & MFLAGCAP)) {
                    pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
                }
            }
        }
    }
    
    // no legal moves, either checkmate or stalemate
    if(Legal == 0) {
        if(InCheck) {
            return -INFINITE + pos->ply;
        } else {
            return 0;
        }
    }
    
    ASSERT(alpha>=OldAlpha);
    
    // store in hash table w exact tag or alpha tag
    if(alpha != OldAlpha) {
        StoreHashEntry(pos, BestMove, BestScore, HFEXACT, depth);
    } else {
        // did not beat alpha, but still best move in search
        StoreHashEntry(pos, BestMove, alpha, HFALPHA, depth);
    }
    
    return alpha;
}

void SearchPosition(BoardState *pos, Search *info) {
    
    int bestMove = NOMOVE;
    int bestScore = -INFINITE;
    int currentDepth = 0;
    int pvMoves = 0;
    int pvNum = 0;
    
    ClearForSearch(pos,info);
    
    // iterative deepening: https://www.chessprogramming.org/Iterative_Deepening
    for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
        
        // search for best score
        rootDepth = currentDepth;
        bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE);
        
        // break if out of time
        if(info->stopped == TRUE) {
            break;
        }
        
        // set best move to Principal Variation
        pvMoves = GetPVLine(currentDepth, pos);
        bestMove = pos->PVArray[0];
        
        // UCI and XBoard prints
        if(info->GAME_MODE == UCIMODE) {
            printf("info score cp %d depth %d nodes %ld time %d ",
                   bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
        } else if(info->GAME_MODE == XBOARDMODE && info->POST_THINKING == TRUE) {
            printf("%d %d %d %ld ",
                   currentDepth,bestScore,(GetTimeMs()-info->starttime)/10,info->nodes);
        } else if(info->POST_THINKING == TRUE) {
            printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
                   bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
        }
        if(info->GAME_MODE == UCIMODE || info->POST_THINKING == TRUE) {
            pvMoves = GetPVLine(currentDepth, pos);
            if((!info->GAME_MODE) == XBOARDMODE) {
                printf("pv");
            }
            for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
                printf(" %s",PrMove(pos->PVArray[pvNum]));
            }
            printf("\n");
        }
    }
    
    if(info->GAME_MODE == UCIMODE) {
        printf("bestmove %s\n",PrMove(bestMove));
    } else if(info->GAME_MODE == XBOARDMODE) {
        printf("move %s\n",PrMove(bestMove));
        MakeMove(pos, bestMove);
    } else {
        printf("\n\nSECS makes move %s.",PrMove(bestMove));
        MakeMove(pos, bestMove);
        PrintBoard(pos);
    }
}
