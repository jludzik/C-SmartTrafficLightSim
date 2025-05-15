#include <stdio.h>
#include "rw_logic.h"
#include "traffic.h"

#define ERROR_RW_ARGS 1
#define ERROR_RW_GETCOMMAND 2
#define ERROR_SIM_INIT_FAILED 3
#define ERROR_SIM_STEP_FAILED 4
#define ERROR_RW_CREATE_LOG 5

int main(int argc, char** argv)
{
    if(!(rw_checkInputArgs(argc, argv))) return ERROR_RW_ARGS;

    int carNumber = 0;
    int simSteps = 0;
    
    Car* parkingHead = rw_getCommands(argv[1],&carNumber,&simSteps);
    if(parkingHead == NULL) return ERROR_RW_GETCOMMAND;
    else printf(">Input file read successfully\n");

    if(!traffic_simInit(&parkingHead)) return ERROR_SIM_INIT_FAILED;
    else printf(">Simulation initialized successfully\n");

    Car* outputHead = NULL;
    for(int i=0;i<simSteps;i++)
    {
        if(!(traffic_simStep(&parkingHead, &outputHead)))
        {
            printf(">>Step %d failed\n",i);
            return ERROR_SIM_STEP_FAILED;
        }
    }

    if(!rw_createOutLog(argv[2],outputHead,simSteps)) return ERROR_RW_CREATE_LOG;
    else printf(">Result file created successfully\n");

    return 0;
}