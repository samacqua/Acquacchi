// board.c

#include "stdio.h"
#include "defs.h"

int CheckBoard(const S_BOARD *pos) {
     
    int sq64,t_piece,t_pce_num,sq120,pcount;
    
    U64 t_whitebb = 0ULL;
    U64 t_blackbb = 0ULL;
    U64 t_pawnbb = 0ULL;
    U64 t_knightbb = 0ULL;
    U64 t_bishopbb = 0ULL;
    U64 t_rookbb = 0ULL;
    U64 t_queenbb = 0ULL;
    U64 t_kingbb = 0ULL;
    
    t_whitebb = pos->bitboards[0];
    t_blackbb = pos->bitboards[1];
    t_pawnbb = pos->bitboards[2];
    t_knightbb = pos->bitboards[3];
    t_bishopbb = pos->bitboards[4];
    t_rookbb = pos->bitboards[5];
    t_queenbb = pos->bitboards[6];
    t_kingbb = pos->bitboards[7];
    
    // check bitboards
    ASSERT(t_whitebb - pos->bitboards[0]==0);
    ASSERT(t_blackbb - pos->bitboards[1]==0);
    ASSERT(t_pawnbb - pos->bitboards[2]==0);
    ASSERT(t_knightbb - pos->bitboards[3]==0);
    ASSERT(t_bishopbb - pos->bitboards[4]==0);
    ASSERT(t_rookbb - pos->bitboards[5]==0);
    ASSERT(t_queenbb - pos->bitboards[6]==0);
    ASSERT(t_kingbb - pos->bitboards[7]==0);
    
    // checking other values
    
    t_whitebb |= (1ULL << SQ64(D2));
    t_pawnbb |= (1ULL << SQ64(D2));
    t_pawnbb |= (1ULL << SQ64(H2));
    PrintBitBoard(t_whitebb&t_pawnbb);
    PrintBitBoard(t_pawnbb);
    
    ASSERT(pos->side==WHITE || pos->side==BLACK);
    ASSERT(GeneratePosKey(pos)==pos->posKey);
//    ASSERT(pos->enPas==NO_SQ || ( RanksBrd[pos->enPas]==RANK_6 && pos->side == WHITE)
//         || ( RanksBrd[pos->enPas]==RANK_3 && pos->side == BLACK));
         
    return TRUE;
}

void UpdateBitBoards(S_BOARD *pos) {
    
    int piece,sq,index,color;
    
    for(index = 0; index < BRD_SQ_NUM; ++index) {
        sq = SQ64(index);
        piece = pos->pieces[index];
        if(piece!=OFFBOARD && piece!= EMPTY) {
            color = PieceCol[piece];
            
            pos->material[color] += PieceVal[piece];
            
            SETBIT(pos->bitboards[color], sq);  // sets white and black bb
            
            if (piece==wP || piece==bP) {
                SETBIT(pos->bitboards[2], sq);
            } else if (piece==wN || piece==bN) {
                SETBIT(pos->bitboards[3], sq);
            } else if (piece==wB || piece==bB) {
                SETBIT(pos->bitboards[4], sq);
            } else if (piece==wR || piece==bR) {
                SETBIT(pos->bitboards[5], sq);
            } else if (piece==wQ || piece==bQ) {
                SETBIT(pos->bitboards[6], sq);
            } else if (piece==wK || piece==bK) {
                SETBIT(pos->bitboards[7], sq);
            }
        }
    }
}

int ParseFen(char *fen, S_BOARD *pos) {

    ASSERT(fen!=NULL);
    ASSERT(pos!=NULL);

    int  rank = RANK_8;
    int  file = FILE_A;
    int  piece = 0;
    int  count = 0;
    int  i = 0;
    int  sq64 = 0;
    int  sq120 = 0;

    ResetBoard(pos);
    
    // Board configuration

    while ((rank >= RANK_1) && *fen) {
        count = 1;
        switch (*fen) {
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

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                piece = EMPTY;
                count = *fen - '0'; // difference in ASCII values gives integer number
                break;

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
    
    // Side to Move
    pos->side = (*fen == 'w') ? WHITE : BLACK;
    fen += 2;
    
    // Castle permissions
    for (i = 0; i < 4; i++) {
        if (*fen == ' ') {
            break;
        }
        switch(*fen) {
            case 'K': pos->castlePerm |= WKCA; break;
            case 'Q': pos->castlePerm |= WQCA; break;
            case 'k': pos->castlePerm |= BKCA; break;
            case 'q': pos->castlePerm |= BQCA; break;
            default:         break;
        }
        fen++;
    }
    fen++;
    
    ASSERT(pos->castlePerm>=0 && pos->castlePerm <= 15);
    
    // En passant
    if (*fen != '-') {
        file = fen[0] - 'a';
        rank = fen[1] - '1';
        
        ASSERT(file>=FILE_A && file <= FILE_H);
        ASSERT(rank>=RANK_1 && rank <= RANK_8);
        
        pos->enPas = FR2SQ(file,rank);
    }
    
    // generate hash key
    pos->posKey = GeneratePosKey(pos);
    
    return 0;
}

void ResetBoard(S_BOARD *pos) {

    int index = 0;
    
    for(index = 0; index < BRD_SQ_NUM; ++index) {
        pos->pieces[index] = OFFBOARD;
    }
    
    for(index = 0; index < 64; ++index) {
        pos->pieces[SQ120(index)] = EMPTY;
    }
    
    for(index=0; index < 8; index++) {
        pos->bitboards[index] = 0ULL;
    }
    
    pos->side = BOTH;
    pos->enPas = NO_SQ;
    pos->fiftyMove = 0;
    
    pos->searchPly = 0;
    pos->movePly = 0;
    
    pos->castlePerm = 0;
    
    pos->posKey = 0ULL;
    
}

void PrintBoard(const S_BOARD *pos) {
    
    int sq,file,rank,piece;
    
    printf("\nGame Board:\n\n");
    
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
    printf("side:%c\n",SideChar[pos->side]);
    printf("enPas:%d\n",pos->enPas);
    printf("castle:%c%c%c%c\n",
            pos->castlePerm & WKCA ? 'K' : '-',
            pos->castlePerm & WQCA ? 'Q' : '-',
            pos->castlePerm & BKCA ? 'k' : '-',
            pos->castlePerm & BQCA ? 'q' : '-'
            );
    printf("PosKey:%llX\n",pos->posKey);
}
