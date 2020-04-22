// evaluation.c

#include "stdio.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

const int PawnIsolated = -10;
const int PawnPassed[8] = { 0, 5, 10, 20, 35, 60, 100, 200 }; 
const int RookOpenFile = 10;
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int BishopPair = 30;

// higher values in piece table gives it higher score. So, pieces tend to seek these squares: https://www.chessprogramming.org/Piece-Square_Tables
const int PawnTable[64] = {
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int KnightTable[64] = {
0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0		
};

const int BishopTable[64] = {
0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	
};

const int RookTable[64] = {
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
0	,	0	,	5	,	10	,	10	,	5	,	0	,	0		
};

const int KingTableO[64] = {
    0    ,    5    ,    5    ,    -10    ,    -10    ,    0    ,    10    ,    5    ,
    -30    ,    -30    ,    -30    ,    -30    ,    -30    ,    -30    ,    -30    ,    -30    ,
    -50    ,    -50    ,    -50    ,    -50    ,    -50    ,    -50    ,    -50    ,    -50    ,
    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,
    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,
    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,
    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,
    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70    ,    -70
};

const int KingTableE[64] = {
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
	0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
	0	,	10	,	20	,	40	,	40	,	20	,	10	,	0	,
	0	,	10	,	20	,	20	,	20	,	20	,	10	,	0	,
	-10,	0	,	10	,	10	,	10	,	10	,	0	,	-10	,
	-50	,	-10	,	0	,	0	,	0	,	0	,	-10	,	-50	
};

// from sjeng 11.2: https://github.com/gcp/sjeng
// assuming best play, material counts where check mate is impossible
int IsMaterialDraw(const BoardState *pos) {

	ASSERT(BoardListsConsistent(pos));
	
    if (!pos->pceNum[wR] && !pos->pceNum[bR] && !pos->pceNum[wQ] && !pos->pceNum[bQ]) {
	  if (!pos->pceNum[bB] && !pos->pceNum[wB]) {
	      if (pos->pceNum[wN] < 3 && pos->pceNum[bN] < 3) {  return TRUE; }
	  } else if (!pos->pceNum[wN] && !pos->pceNum[bN]) {
	     if (abs(pos->pceNum[wB] - pos->pceNum[bB]) < 2) { return TRUE; }
	  } else if ((pos->pceNum[wN] < 3 && !pos->pceNum[wB]) || (pos->pceNum[wB] == 1 && !pos->pceNum[wN])) {
	    if ((pos->pceNum[bN] < 3 && !pos->pceNum[bB]) || (pos->pceNum[bB] == 1 && !pos->pceNum[bN]))  { return TRUE; }
	  }
	} else if (!pos->pceNum[wQ] && !pos->pceNum[bQ]) {
        if (pos->pceNum[wR] == 1 && pos->pceNum[bR] == 1) {
            if ((pos->pceNum[wN] + pos->pceNum[wB]) < 2 && (pos->pceNum[bN] + pos->pceNum[bB]) < 2)	{ return TRUE; }
        } else if (pos->pceNum[wR] == 1 && !pos->pceNum[bR]) {
            if ((pos->pceNum[wN] + pos->pceNum[wB] == 0) && (((pos->pceNum[bN] + pos->pceNum[bB]) == 1) || ((pos->pceNum[bN] + pos->pceNum[bB]) == 2))) { return TRUE; }
        } else if (pos->pceNum[bR] == 1 && !pos->pceNum[wR]) {
            if ((pos->pceNum[bN] + pos->pceNum[bB] == 0) && (((pos->pceNum[wN] + pos->pceNum[wB]) == 1) || ((pos->pceNum[wN] + pos->pceNum[wB]) == 2))) { return TRUE; }
        }
    }
  return FALSE;
}

#define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK])

// higher value = better for white, more negative = better for black
int PositionEvaluation(const BoardState *pos) {

	ASSERT(BoardListsConsistent(pos));

	int pce;
	int pceNum;
	int sq;
    // score begins as just material count, position evaluation added
	int score = pos->material[WHITE] - pos->material[BLACK];
	
    // if (assuming best play) material draw, then drawn position
	if(!pos->pceNum[wP] && !pos->pceNum[bP] && IsMaterialDraw(pos) == TRUE) {
		return 0;
	}
    
    // add piece square table values and penalty for isolated pawns and bonus for passed pawns
	pce = wP;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += PawnTable[SQ64(sq)];	
		
		if( (IsolatedBBMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("wP Iso:%s\n",PrSq(sq));
			score += PawnIsolated;
		}
		
		if( (WhitePassedBBMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("wP Passed:%s\n",PrSq(sq));
			score += PawnPassed[RanksBrd[sq]];
		}
		
	}
    
	pce = bP;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR(SQ64(sq))>=0 && MIRROR(SQ64(sq))<=63);
		score -= PawnTable[MIRROR(SQ64(sq))];
		
		if( (IsolatedBBMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
			//printf("bP Iso:%s\n",PrSq(sq));
			score -= PawnIsolated;
		}
		
		if( (BlackPassedBBMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
			//printf("bP Passed:%s\n",PrSq(sq));
			score -= PawnPassed[7 - RanksBrd[sq]];
		}
	}	
	
    // add piece square table values
	pce = wN;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += KnightTable[SQ64(sq)];
	}	

	pce = bN;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR(SQ64(sq))>=0 && MIRROR(SQ64(sq))<=63);
		score -= KnightTable[MIRROR(SQ64(sq))];
	}			
	
    // add piece square table values and bonus for bishop pair
	pce = wB;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += BishopTable[SQ64(sq)];
	}
    if(pos->pceNum[wB] >= 2) score += BishopPair;

	pce = bB;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR(SQ64(sq))>=0 && MIRROR(SQ64(sq))<=63);
		score -= BishopTable[MIRROR(SQ64(sq))];
	}
    if(pos->pceNum[bB] >= 2) score -= BishopPair;

    // add piece square table values and bonuses for open files and semi-open files
	pce = wR;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		score += RookTable[SQ64(sq)];
		
		ASSERT(FileRankValid(FilesBrd[sq]));
		
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score += RookOpenFile;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			score += RookSemiOpenFile;
		}
	}	

	pce = bR;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR(SQ64(sq))>=0 && MIRROR(SQ64(sq))<=63);
		score -= RookTable[MIRROR(SQ64(sq))];
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= RookOpenFile;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= RookSemiOpenFile;
		}
	}	
	
    // add piece square table values and bonuses for open files and semi-open files
	pce = wQ;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score += QueenOpenFile;
		} else if(!(pos->pawns[WHITE] & FileBBMask[FilesBrd[sq]])) {
			score += QueenSemiOpenFile;
		}
	}	

	pce = bQ;	
	for(pceNum = 0; pceNum < pos->pceNum[pce]; ++pceNum) {
		sq = pos->pieceList[pce][pceNum];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
		ASSERT(FileRankValid(FilesBrd[sq]));
		if(!(pos->pawns[BOTH] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenOpenFile;
		} else if(!(pos->pawns[BLACK] & FileBBMask[FilesBrd[sq]])) {
			score -= QueenSemiOpenFile;
		}
	}	

    pce = wK;
	sq = pos->pieceList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(SQ64(sq)>=0 && SQ64(sq)<=63);
	
    // add piece square table depending on if endgame or not
	if( (pos->material[BLACK] <= ENDGAME_MAT) ) {
		score += KingTableE[SQ64(sq)];
	} else {
		score += KingTableO[SQ64(sq)];
	}
	
	pce = bK;
	sq = pos->pieceList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(MIRROR(SQ64(sq))>=0 && MIRROR(SQ64(sq))<=63);
	
	if( (pos->material[WHITE] <= ENDGAME_MAT) ) {
		score -= KingTableE[MIRROR(SQ64(sq))];
	} else {
		score -= KingTableO[MIRROR(SQ64(sq))];
	}
    
    // flip value if black bc trying to maximize score
	if(pos->side == WHITE) {
		return score;
	} else {
		return -score;
	}	
}


















