#include "predictor.h"

#include <stdio.h>
#include <stdlib.h>

#define BUFSIZE 200

int main(int argc, const char* argv[])
{
    FILE* fp;
    char buf[BUFSIZE];
    branch_record_c br;
    int target_addr = 0x100;
    bool taken;
    PREDICTOR p;
    uint predicted_addr;
    bool predicted_taken;

    if(argc < 2)
    {
        printf("No filename provided\n");
        return -1;
    }

    printf("Opening file: %s\n", argv[1]);
    fp = fopen(argv[1], "r");

    if(!fp)
    {
        printf("Error reading file\n");
        return -1;
    }

    printf("PC\tCond\tCall\tRet\tP_Tgt\tP_Taken\tTgt\tTaken\tTaken?\tTgt?\n");
    while(fgets(buf, BUFSIZE - 1, fp) != NULL)
    {
        sscanf(buf, "%x%d%d%d%d%x\n", 
                &br.instruction_addr, 
                (int*)&br.is_conditional, 
                (int*)&br.is_call, 
                (int*)&br.is_return,
                (int*)&taken,
                &target_addr);

        printf("addr: %x\n", target_addr);
        predicted_taken = p.get_prediction(&br, 0, &predicted_addr);

        printf("%X\t%d\t%d\t%d\t%X\t%d\t%X\t%d\t%d\t%d\n",
                br.instruction_addr,
                br.is_conditional,
                (int)br.is_call,
                (int)br.is_return,
                predicted_addr,
                (int)predicted_taken,
                target_addr,
                taken,
                (int)(taken == predicted_taken),
                (int)(target_addr == predicted_addr));


        p.update_predictor(&br, 0, taken, target_addr);

    }

}
