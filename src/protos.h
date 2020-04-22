#ifndef PROTOS_H
#define PROTOS_H

#include "stdlib.h"
#include "stdio.h"

#define POP(b) BitScanForward(b)
#define CNT(b) CountBits(b)

// MARK: FUNCTION PROTOTYPES

// init.c
extern void Initialize();
extern void InitHashTable(HashTable *table, const int MB);

// bitboards.c
extern void PrintBitBoard(U64 bb);
extern int BitScanForward(U64 *bb);
extern int CountBits(U64 b);

// hash.c
extern U64 GenerateHashKey(const BoardState *pos);
extern void StoreHashEntry(BoardState *pos, const int move, int score, const int flags, const int depth);
extern int ProbeHashEntry(BoardState *pos, int *move, int *score, int alpha, int beta, int depth);
extern int ProbePVMove(const BoardState *pos);
extern int GetPVLine(const int depth, BoardState *pos);
extern void ClearHashTable(HashTable *table);

// board.c
extern void ResetBoard(BoardState *pos);
extern int ParseFEN(char *fen, BoardState *pos);
extern void PrintBoard(const BoardState *pos);
extern void UpdateMaterialLists(BoardState *pos);
extern void MirrorBoard(BoardState *pos);

//validate.c
extern int BoardListsConsistent(const BoardState *pos);
extern int PieceListValid(const BoardState *pos);
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceEmpty(const int pce);
extern int PieceValid(const int pce);
extern void MirrorEvalTest(BoardState *pos);
extern int SqIs120(const int sq);
extern int PieceEmptyOrOffbrd(const int pce);
extern int MoveListValid(const MoveList *list,  const BoardState *pos);
extern void DebugBoardState(BoardState *pos, Search *info);

// movegen.c
extern int IsSqAttacked(const int sq, const int side, const BoardState *pos);
extern void GenerateAllMoves(const BoardState *pos, MoveList *list);
extern void GenerateAllCaptureMoves(const BoardState *pos, MoveList *list);
extern int MoveExists(BoardState *pos, const int move);
extern void InitMvvLva();

// makemove.c
extern int MakeMove(BoardState *pos, int move);
extern void TakeBackMove(BoardState *pos);
extern void MakeNullMove(BoardState *pos);
extern void TakeBackNullMove(BoardState *pos);

// perft.c
extern void PerftTest(int depth, BoardState *pos);

// search.c
extern void SearchPosition(BoardState *pos, Search *info);

// misc.c
extern int GetTimeMs();
extern void ParseGUIInput(Search *info);

// evaluation.c
extern int PositionEvaluation(const BoardState *pos);
extern void MirrorEvalTest(BoardState *pos) ;

// protocols.c
extern void Uci_Loop(BoardState *pos, Search *info);
extern void XBoard_Loop(BoardState *pos, Search *info);
extern void Console_Loop(BoardState *pos, Search *info);
extern char *PrMove(const int move);
extern char *PrSq(const int sq);
extern void PrintMoveList(const MoveList *list);
extern int ParseMove(char *ptrChar, BoardState *pos);

#endif
