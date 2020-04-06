// secs.c

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "defs.h"
#include "protos.h"
#include "data.h"

#define WAC1 "r1b1k2r/ppppnppp/2n2q2/2b5/3NP3/2P1B3/PP3PPP/RN1QKB1R w KQkq - 0 1"
#define PERFT "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

int main() {
    
    Initialize();
    
    BoardState pos[1];
    Search info[1];
    info->quit = FALSE;
    pos->hashTable->hashEntry = NULL;
    InitHashTable(pos->hashTable, 64);
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    printf("Welcome to SECS - a Simple Engine for Chess by Sam! Type 'secs' for terminal mode...\n");
    
    char line[256];
    while (TRUE) {
        memset(&line[0], 0, sizeof(line));
        
        fflush(stdout);
        if (!fgets(line, 256, stdin))
            continue;
        if (line[0] == '\n')
            continue;
        if (!strncmp(line, "uci",3)) {
            Uci_Loop(pos, info);
            if(info->quit == TRUE) break;
            continue;
        } else if (!strncmp(line, "xboard",6))	{
            XBoard_Loop(pos, info);
            if(info->quit == TRUE) break;
            continue;
        } else if (!strncmp(line, "secs",4))	{
            Console_Loop(pos, info);
            if(info->quit == TRUE) break;
            continue;
        } else if(!strncmp(line, "quit",4))	{
            break;
        }
    }
    
    free(pos->hashTable->hashEntry);
    
    return 0;
}
