#include "common.h"

#define SHMSZ1 800
#define SHMSZ2 100
#define SHMSZ3 100
#define QUEUE_SIZE 5

void Sync_API(client_request *q, int *queue_full, sem_t *semlock, int num_req)
{
	int req_count = 0; //keeps track of the number of requests made by process
	
    	if(!(*queue_full))
    	*queue_full = 0;	// indicates that the queue has something inside it

	int *queue_counter = queue_full + 4;

	int *queue_fifo_id_counter = queue_full + 8; // to keep track of global fifo_priority ID
	int inserted_flag;

	while(req_count < num_req)
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
