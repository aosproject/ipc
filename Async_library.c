#include "common.h"

#define SHMSZ1 800
#define SHMSZ2 100
#define SHMSZ3 100
#define QUEUE_SIZE 5

void Async_API(client_request *q, int *queue_full, sem_t *semlock, int num_req)
{
	int req_count = 0; //keeps track of the number of requests made by process
	
	if(!(*queue_full))
    	*queue_full = 0;	// indicates that the queue has something inside it

	int *queue_counter = queue_full + 4;

	int *queue_fifo_id_counter = queue_full + 8; // to keep track of global fifo_priority ID

	client_request *q_index;

	while(req_count < num_req)
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


