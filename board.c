// board.c

#include "stdio.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

void UpdateMaterialLists(BoardState *pos) {
    
    int piece,sq,index,color;
    
    // loop through all squares on the board
    for(index = 0; index < BRD_SQ_NUM; ++index) {
        sq = index;
        piece = pos->pieces[index];
        ASSERT(PieceEmptyOrOffbrd(piece));
        
        // if there is a piece on the square
        if(piece!=OFFBOARD && piece!= EMPTY) {
            color = PieceCol[piece];
            ASSERT(SideValid(color));
            
            if( IsPieceBig[piece] == TRUE) pos->bigPce[color]++;
            if( IsPieceMin[piece] == TRUE) pos->minPce[color]++;
            if( IsPieceBig[piece] == TRUE) pos->majPce[color]++;
            
            pos->material[color] += PieceVal[piece];
            
            ASSERT(pos->pceNum[piece] < 10 && pos->pceNum[piece] >= 0);
            
            // add the piece and square to end of piece list, increment count of piece list
            pos->pieceList[piece][pos->pceNum[piece]++] = sq;
            
            if(piece==wK) pos->KingSq[WHITE] = sq;
            if(piece==bK) pos->KingSq[BLACK] = sq;
            
            // if pawn update set bit on bitboard
            if(piece==wP) {
                SETBIT(pos->pawns[WHITE],SQ64(sq));
                SETBIT(pos->pawns[BOTH],SQ64(sq));
            } else if(piece==bP) {
                SETBIT(pos->pawns[BLACK],SQ64(sq));
                SETBIT(pos->pawns[BOTH],SQ64(sq));
            }
        }
    }
}

// Forsyth-Edwards Notation: https://www.chessprogramming.org/Forsyth-Edwards_Notation
// starting FEN: "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
int ParseFEN(char *fen, BoardState *pos) {
    
    ASSERT(fen!=NULL);
    ASSERT(pos!=NULL);
    
    int  rank = RANK_8;
    int  file = FILE_A;
    int  piece = 0; // != 0 if find a piece
    int  count = 0; // # of empty spaces
    int  i = 0;
    int  sq64 = 0;
    int  sq120 = 0;
    
    ResetBoard(pos);
    
    // rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR
    while ((rank >= RANK_1) && *fen) {
        count = 1;
        switch (*fen) {
                
                // found a piece
            case 'p': piece = bP; break;
            case 'r': piece = bR; break;
            case 'n': piece = bN; break;
            case 'b': piece = bB; break;
            case 'k': piece = bK; break;
            case 'q': piece = bQ; break;
            case 'P': piece = wP; break;
            case 'R': piece = wR; break;
            case 'N': piece = wN; break;
            case 'B': piece = wB; break;
            case 'K': piece = wK; break;
            case 'Q': piece = wQ; break;
                
                // # empty spaces
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                // difference is ascii values is value. ex: '1'-'0' = 1
                count = *fen - '0';
                break;
                
                // new rank or end of piece setup
            case '/':
            case ' ':
                rank--;
                file = FILE_A;
                fen++;
                continue;
                
            default:
                printf("FEN error \n");
                return -1;
        }
        
        // loop through board, add found piece to its position
        for (i = 0; i < count; i++) {
            sq64 = rank * 8 + file;
            sq120 = SQ120(sq64);
            if (piece != EMPTY) {
                pos->pieces[sq120] = piece;
            }
            file++;
        }
        fen++;
    }
    
    ASSERT(*fen == 'w' || *fen == 'b');
    
    // if character is white, then current side is white
    pos->side = (*fen == 'w') ? WHITE : BLACK;
    fen += 2;
    
    // add castling permissions
    for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
        switch(*fen) {
            case 'K': pos->castlePerm |= WKCA; break;
            case 'Q': pos->castlePerm |= WQCA; break;
            case 'k': pos->castlePerm |= BKCA; break;
            case 'q': pos->castlePerm |= BQCA; break;
            default:	     break;
        }
        fen++;
    }
    fen++;
    
    ASSERT(pos->castlePerm>=0 && pos->castlePerm <= 15);
    
    // set en passant square
    if (*fen != '-') {
        // difference in ascii values yields file and rank
        file = fen[0] - 'a';
        rank = fen[1] - '1';
        
        ASSERT(file>=FILE_A && file <= FILE_H);
        ASSERT(rank>=RANK_1 && rank <= RANK_8);
        
        pos->enPas = FR2SQ(file,rank);
    }
    
    pos->hashKey = GenerateHashKey(pos);
    
    UpdateMaterialLists(pos);
    
    return 0;
}

// empty board and reset all position attributes
void ResetBoard(BoardState *pos) {
    
    int index = 0;
    
    // set 64 middle squares to empty and outside squares to offboard
    for(index = 0; index < BRD_SQ_NUM; ++index) {
        pos->pieces[index] = OFFBOARD;
    }
    for(index = 0; index < 64; ++index) {
        pos->pieces[SQ120(index)] = EMPTY;
    }
    
    // empty piece categories
    for(index = 0; index < 2; ++index) {
        pos->bigPce[index] = 0;
        pos->majPce[index] = 0;
        pos->minPce[index] = 0;
        pos->material[index] = 0;
    }
    
    // empty pawn count
    for(index = 0; index < 3; ++index) {
        pos->pawns[index] = 0ULL;
    }
    
    // empty piece count
    for(index = 0; index < 13; ++index) {
        pos->pceNum[index] = 0;
    }
    
    // reset rest of position attributes
    pos->KingSq[WHITE] = pos->KingSq[BLACK] = NO_SQ;
    
    pos->side = BOTH;
    pos->enPas = NO_SQ;
    pos->fiftyMove = 0;
    
    pos->ply = 0;
    pos->hisPly = 0;
    
    pos->castlePerm = 0;
    
    pos->hashKey = 0ULL;
}

void PrintBoard(const BoardState *pos) {
    
    int sq,file,rank,piece;
    
    printf("\nGame Board:\n\n");
    
    // Iterate backwards so A rank on bottom when printing board
    for(rank = RANK_8; rank >= RANK_1; rank--) {
        printf("%d  ",rank+1);
        for(file = FILE_A; file <= FILE_H; file++) {
            sq = FR2SQ(file,rank);
            piece = pos->pieces[sq];
            printf("%3c",PceChar[piece]);
        }
        printf("\n");
    }
    printf("\n   ");
    
    for(file = FILE_A; file <= FILE_H; file++) {
        printf("%3c",'a'+file);
    }
    printf("\n");
    
    // print other position information
    printf("side:%c\n",SideChar[pos->side]);
    printf("enPas:%d\n",pos->enPas);
    printf("castle:%c%c%c%c\n",
           pos->castlePerm & WKCA ? 'K' : '-',
           pos->castlePerm & WQCA ? 'Q' : '-',
           pos->castlePerm & BKCA ? 'k' : '-',
           pos->castlePerm & BQCA ? 'q' : '-'
           );
    printf("hashKey:%llX\n",pos->hashKey);
}

// set whats in a8 to a1 and vice versa, a7->a2, b8->b1, etc...
// used to check that evaluation stays symmetrical
void MirrorBoard(BoardState *pos) {
    
    int tempPiecesArray[64];
    int tempSide = pos->side^1;
    int SwapPiece[13] = { EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK };
    int tempCastlePerm = 0;
    int tempEnPas = NO_SQ;
    
    int sq;
    int tp;
    
    // switch castling permissions
    if (pos->castlePerm & WKCA) tempCastlePerm |= BKCA;
    if (pos->castlePerm & WQCA) tempCastlePerm |= BQCA;
    if (pos->castlePerm & BKCA) tempCastlePerm |= WKCA;
    if (pos->castlePerm & BQCA) tempCastlePerm |= WQCA;
    
    // mirror en passant square
    if (pos->enPas != NO_SQ)  {
        tempEnPas = SQ120(MIRROR(SQ64(pos->enPas)));
    }
    
    // get pieces on mirrored board
    for (sq = 0; sq < 64; sq++) {
        tempPiecesArray[sq] = pos->pieces[SQ120(MIRROR(sq))];
    }
    
    ResetBoard(pos);
    
    for (sq = 0; sq < 64; sq++) {
        tp = SwapPiece[tempPiecesArray[sq]];
        pos->pieces[SQ120(sq)] = tp;
    }
    
    // switch side, castle permissions, and en passant square
    pos->side = tempSide;
    pos->castlePerm = tempCastlePerm;
    pos->enPas = tempEnPas;
    
    pos->hashKey = GenerateHashKey(pos);
    UpdateMaterialLists(pos);
    
    ASSERT(BoardListsConsistent(pos));
}
