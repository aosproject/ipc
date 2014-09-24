#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>


typedef struct client_request{
    int PID;
    int reqID;
    int input;
    int output;
    int fifo_priority;
    int valid;
    int full;
    int reserved0, reserved1, reserved2;
} client_request;
