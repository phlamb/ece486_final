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

    //statistics storage
    int call_depth = 0;
    int max_call_depth = 0;
    int min_call_depth = 0;

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

    while(fgets(buf, BUFSIZE - 1, fp) != NULL)
    {
        sscanf(buf, "%x%d%d%d%d%x\n", 
                &br.instruction_addr, 
                (int*)&br.is_conditional, 
                (int*)&br.is_call, 
                (int*)&br.is_return,
                (int*)&taken,
                &target_addr);

        if(br.is_call)
            call_depth++;
        else if(br.is_return)
            call_depth--;

        if(call_depth > max_call_depth)
            max_call_depth = call_depth;
        if(call_depth < min_call_depth)
            min_call_depth = call_depth;

        //printf("%d\n",call_depth);

    }
    printf("Max Call Depth: %d\n", max_call_depth);
    printf("Min Call Depth: %d\n", min_call_depth);
}
