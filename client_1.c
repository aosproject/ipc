#include "common.h"

#define SHMSZ1 800
#define SHMSZ2 100
#define SHMSZ3 100
#define REQ_LIMIT 10
#define QUEUE_SIZE 5
#define PRIO 0

main(int argc, char *argv[])
{
    int mode = 0;

    if(argc < 2) {
	printf(" The IPC mode required is not specified \n");
	exit(0);
    }

    if(argc > 2) {
	printf(" Too many arguments specified \n");
	exit(0);
    }

    if(argc == 2) {
	if(!strcmp(argv[1],"-async"))
		mode = 0;
	else if(!strcmp(argv[1],"-sync"))
		mode = 1;
    }
	
    int shmid1, shmid2, shmid3;
    key_t key1, key2, key3;
    int *shm_full;		//4 slots inside the queue to indicate if the 4 queues are empty/non-empty
    struct client_request *q1;
    sem_t *semlock;
    struct timeval startTime, endTime, diffTime;
    float runTime_msec;

    key1 = 12; // shared memory for the queues
    key2 = 23; // shared memory to signal queue full
    key3 = 34; // shared memory to implement semaphores

    if ((shmid1 = shmget(key1, SHMSZ1, 0666)) < 0) {
        perror("error");
        exit(1);
    }

    if ((shmid2 = shmget(key2, SHMSZ2, 0666)) < 0) {
        perror("error");
        exit(1);
    }

    if ((shmid3 = shmget(key3, SHMSZ3, 0666)) < 0) {
        perror("error");
        exit(1);
    }

    if ((q1 = (client_request*)shmat(shmid1, NULL, 0)) == (client_request *) -1){
        perror("shmat");
        exit(1);
    }

    if ((shm_full = (int*)shmat(shmid2, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    if ((semlock = (sem_t*)shmat(shmid3, NULL, 0)) == (sem_t *) -1) {
        perror("shmat");
        exit(1);
    }

	// Depending on the priority the queue is chosen
	q1 += (5 * PRIO);

	// Depending on the priority the queuefull bit chosen also varies
	int i;
	for(i=0; i<PRIO; i++)
	{
		shm_full++;
	}

	// Depending on the priority the semlock variable chosen also varies
	for(i=0; i<PRIO; i++)
	{
		semlock++;
	}

	gettimeofday(&startTime, NULL);

	// Call to the Synchronous API / Aysnc  API based on the cmd line arguments
	// Sending required number of requests
	if(mode)
		Sync_API(q1, shm_full, semlock, 10);
	else
		Async_API(q1, shm_full, semlock, 10);

    gettimeofday(&endTime, NULL);
    timersub(&endTime, &startTime, &diffTime);
    runTime_msec = (float)((diffTime.tv_sec * 1000.0) + diffTime.tv_usec/1000.0);

    if(mode)
        printf("Client using Sync API with priority %d took %f msec \n", PRIO, runTime_msec);
    else
        printf("Client using Async API with priority %d took %f msec \n", PRIO, runTime_msec);

    shmdt(q1);
    shmdt(shm_full);
    shmdt(semlock);

    sleep(1000);
    exit(0);
}
