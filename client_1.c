#include "common.h"

#define SHMSZ1 800
#define SHMSZ2 100
#define SHMSZ3 100
#define REQ_LIMIT 10
#define QUEUE_SIZE 5
#define PRIO 0

void Sync_API(client_request *q, int *queue_full, sem_t *semlock)
{
	int req_count = 0; //keeps track of the number of requests made by process
	
    	*queue_full = 0;	// indicates that the queue has something inside it
	int *queue_counter = queue_full + 4;

	int *queue_fifo_id_counter = queue_full + 8; // to keep track of global fifo_priority ID
	int inserted_flag;

	while(req_count < REQ_LIMIT)
	{	printf("Request count:%d \n",req_count);
		//semwait:
		//check to see if semaphore locked (sem_wait) -> if locked, waits here
		inserted_flag =0;
		printf("Waiting for lock by %d \n", getpid());
		sem_wait(semlock);
		printf(" Acquired lock by %d \n", getpid());
		
		client_request *q_index = q;
		// Iterate through the elements of the queue based to fill in request
		int i;
		for(i=0; i<QUEUE_SIZE; i++,q_index++)
		{
			if(!q_index->full)
			{
				q_index->PID = getpid();
				q_index->reqID = req_count++; //current request id
				q_index->input = 2;
				q_index->fifo_priority = (*queue_fifo_id_counter)++; //fifo prio based on global counter
				q_index->full = 1;
				if(*queue_full == 0)
				{  printf("changing queue full\n");
					(*queue_full) = 1;
				}
				(*queue_counter) += 1;

				// Release the lock -- TODO
				printf("Released lock after insertion %d\n", getpid());
				sem_post(semlock);
				inserted_flag = 1;
				printf("valid bit of queue index = %d \n",q_index->valid);


				while(!q_index->valid); //Need to spin on the valid bit
				// might need to get lock here
				printf("The result is %d \n", q_index->output);
				(*queue_counter) -= 1;
				if((*queue_counter) == 0)
				{
					(*queue_full) = 0;
				}
				// Release the queue lock held (sem_post)-- TODO
				q_index->full = 0;
				q_index->valid = 0;
				break;	

			}

		}
		// Release the queue lock -- TODO
		if(inserted_flag == 0)
		{
			printf("Released lock outside for loop %d\n", getpid());
			sem_post(semlock);
			//goto semwait;
		}
	}

}

void Async_API(client_request *q, int *queue_full, sem_t *semlock)
{
	int req_count = 0; //keeps track of the number of requests made by process
	
	if(!(*queue_full))
    	*queue_full = 0;	// indicates that the queue has something inside it

	int *queue_counter = queue_full + 4;

	int *queue_fifo_id_counter = queue_full + 8; // to keep track of global fifo_priority ID

	client_request *q_index;

	while(req_count < REQ_LIMIT)
	{	//printf("Request count:%d \n",req_count);
		//semwait:
		// check to see if semaphore locked (sem_wait) -- TODO
		printf("Waiting for lock by %d \n", getpid());
		sem_wait(semlock);
		printf(" Acquired lock by %d \n", getpid());

		q_index = q;
		if(*queue_counter < QUEUE_SIZE) // => Queue is not full yet
		{
			// Iterate through the elements of the queue based to fill in request
			int i;
			for(i=0; i<QUEUE_SIZE; i++,q_index++)
			{
				if(!q_index->full)
				{
					q_index->PID = getpid();
					q_index->reqID = req_count++; //current request id
					q_index->input = 2;
					q_index->fifo_priority = (*queue_fifo_id_counter)++; //fifo prio based on global counter
					q_index->full = 1;
					if(*queue_full == 0)
					{  printf("changing queue full\n");
						(*queue_full) = 1;
					}
					(*queue_counter) += 1;
					//req_count++;
					// Release the lock -- TODO
					printf("valid bit of queue index = %d \n",q_index->valid);
					printf("Released lock after insertion %d\n", getpid());
					sem_post(semlock);
					//inserted_flag = 1;
					//break;
				}
			}
		}
		else if(*queue_counter == QUEUE_SIZE) // => Queue is full
		{
			printf("Released lock when queue ful %d\n", getpid());
			sem_post(semlock);
			//printf("Queue is Full! \n");
			q_index = q;
			// Iterate through the elements of the queue and check for any complete requests based on client PID
			int i;
			for(i=1; i<QUEUE_SIZE; i++,q_index++)
			{			
				if(q_index->PID == getpid())
				{
					if(q_index->valid == 1)
					{
						// might need to get lock here
						sem_wait(semlock);
						printf("The result is %d \n", q_index->output);
						(*queue_counter) -= 1;
						if((*queue_counter) == 0)
						{
							(*queue_full) = 0;
						}
						// Release the queue lock held (sem_post)-- TODO
						sem_post(semlock);
						q_index->full = 0;
						q_index->valid = 0;
						//break;
					}
				}

			}

		}
	
	}	

	// All requests enqueued, check for pending responses
	int request_pending;
	do 
	{
		//printf("All request enqueued - waiting for results\n");
		q_index = q;
		request_pending = 0;
		int i;
		for(i=0; i<QUEUE_SIZE; i++,q_index++)
		{
			if(q_index->PID == getpid() && q_index->full==1){
			// My own request which is not serviced yet
				if(!q_index->valid){
					request_pending = 1;
				} 
				else {
					// Its my request which has been serviced
					printf("The result is %d \n", q_index->output);
					q_index->valid = 0;
					q_index->full = 0; // Freeing up the request slot
					(*queue_counter) -= 1;
					if((*queue_counter) == 0)
					{
						(*queue_full) = 0;
					}
				}
			}
		}

	}while(request_pending);
				
}


main()
{
    int shmid1, shmid2, shmid3;
    key_t key1, key2, key3;
    int *shm_full;		//4 slots inside the queue to indicate if the 4 queues are empty/non-empty
    struct client_request *q1;
    sem_t *semlock;

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

	// Call to the Synchronous API
	//Sync_API(q1, shm_full, semlock);
	Async_API(q1, shm_full, semlock);

    shmdt(q1);
    shmdt(shm_full);
    shmdt(semlock);

    //shmctl(shmid1, IPC_RMID, NULL);
    //shmctl(shmid2, IPC_RMID, NULL);

    exit(0);
}
