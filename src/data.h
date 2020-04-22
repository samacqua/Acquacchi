#ifndef DATA_H
#define DATA_H

#include "stdlib.h"
#include "stdio.h"

// MARK: DATA

// MARK: - 1. Moves

// note, 1111 in binary is f in hexadecimal. 0001 is 1, 0010 is 2, 1010 is 10, etc...

/*
 0000 0000 0000 0000 0000 0111 1111 -> From 0x7F
 0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F
 0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF
 0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000
 0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000
 0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF
 0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000
 */

// move = ( ( from ) | ( to << 7 ) | ( cap << 14 ) | ( prom << 20 ) );  where from, to, cap, prom are actual values
// move |= MFLAGEP; move |= MFLAGPS; move |= MFLAGCA; move |= MFLAGCAP; move |= MFLAGPROM;
// is move en passant? (move & MFLAGEP) ? "YES" : "NO" <- same for all other flags

// move is stored in u64int, this is how that int is broken up
// shift necessary to convert from move int to the from square, to square, captured piece, promoted piece

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

// shift necessary to convert from move int to en passant flag, pawn start flag, castle permissions
#define MFLAGEP 0x40000
#define MFLAGPS 0x80000
#define MFLAGCA 0x1000000

// shift necessary to convert from move int to capture flag and promotion flag. tells if captured or promoted, not what was captured or what was promoted to.
#define MFLAGCAP 0x7C000
#define MFLAGPROM 0xF00000

#define NOMOVE 0

// MARK: - 2. Macros

// convert file/rank to 120 based square, 120 -> 64, 64 -> 120
#define FR2SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) )
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])

// bitboard operations
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])

#define IsBQ(p) (IsPieceBishopQueen[(p)])
#define IsRQ(p) (IsPieceRookQueen[(p)])
#define IsKn(p) (IsPieceKnight[(p)])
#define IsKi(p) (IsPieceKing[(p)])

#define MIRROR(sq) (Mirror64Board[(sq)])

// MARK: - 3. External Variable Declarations

extern int Sq120ToSq64[BRD_SQ_NUM];
extern int Sq64ToSq120[64];
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];
extern char PceChar[];
extern char SideChar[];
extern char RankChar[];
extern char FileChar[];

// booleans for piece category
extern int IsPieceBig[13];
extern int IsPieceBig[13];
extern int IsPieceMin[13];
extern int IsPiecePawn[13];

// booleans for piece type
extern int IsPieceKnight[13];
extern int IsPieceKing[13];
extern int IsPieceRookQueen[13];
extern int IsPieceBishopQueen[13];
extern int DoesPieceSlide[13];

extern int PieceVal[13];
extern int PieceCol[13];

extern int FilesBrd[BRD_SQ_NUM];
extern int RanksBrd[BRD_SQ_NUM];

extern int Mirror64Board[64];

// bitboard masks
extern U64 FileBBMask[8];
extern U64 RankBBMask[8];

extern U64 BlackPassedBBMask[64];
extern U64 WhitePassedBBMask[64];
extern U64 IsolatedBBMask[64];

#endif
