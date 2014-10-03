#ifndef _COMMON_DATA
#define _COMMON_DATA_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>


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

extern void Async_API(client_request *q, int *queue_full, sem_t *semlock, int num_req);
extern void Sync_API(client_request *q, int *queue_full, sem_t *semlock, int num_req);

#endif
