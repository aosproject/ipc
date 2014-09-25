#include "common.h"
#include <time.h>
#include <signal.h>

#define SHMSZ1 800 // 800 bytes of shared memory for 4 priority queues
#define SHMSZ2 100  // 100 bytes of shared memory for 4 flags per queue to denote full/empty queue

int call_service(int);
struct client_request *q1,*q2,*q3,*q4; // Four priority queues
int *queue_full; // pointer to indicate the status(full/empty) of each queue
int shmid1, shmid2;

void sig_handler(int signo)
{
    //printf("\n In sig handler \n");
  // SIGINT handler to cleanup the shared memory segment if Ctrl+C pressed
  if (signo == SIGINT){
    
    printf("received SIGINT\n");
    shmdt(q1);
    shmdt(queue_full);
    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);
    exit(0);

  }
  else{
    printf("Caught unknown signal!!! \n");
  }      
}


main()
{
    char c;
    
    key_t key1, key2;
    
    int numQueues = 4, requestsPerQueue = 5, numRequest, i;
    srand(time(NULL)); 

    if (signal(SIGINT, sig_handler) == SIG_ERR)
            printf("\nError catching SIGINT\n");

    //char *shm, *s;

    key1 = 12; // shared memory for the queues
    key2 = 23; // shared memory to signal queue full

    if ((shmid1 = shmget(key1, SHMSZ1, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((shmid2 = shmget(key2, SHMSZ2, IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    if ((q1 = (client_request*)shmat(shmid1, NULL, 0)) == (client_request *) -1) {
        perror("shmat");
        exit(1);
    }

    if ((queue_full = (int*)shmat(shmid2, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    // 40 bytes for 1 request

    // q2, q3, q4 point to head of individual queues
    q2 = q1 + requestsPerQueue; 
    q3 = q2 + requestsPerQueue;
    q4 = q3 + requestsPerQueue;

    client_request *q_init_iterator = q1;
    // **** Initialization of request queue parameters for all queues

    for(i=1; i<numQueues*requestsPerQueue; i++, q_init_iterator++)
    {
        q_init_iterator->PID = 0;       // getpid() -> process id of the calling process
        q_init_iterator->reqID = 0;     // keeps track of the current request number
        q_init_iterator->input = 0;     // input to the server
        q_init_iterator->output = 0;        // result of tjhe service
        q_init_iterator->fifo_priority = 0; // global counter in shared memory to implement priority (set to 0 initially)
        q_init_iterator->valid = 0;     // indicates if result is valid
        q_init_iterator->full = 0;      // indicates if queue slot is used or not
    }

    int *int_iterator = queue_full;
    for(i=0; i<12; i++, int_iterator++)
    {
       *int_iterator = 0;
    }

    // Loop forever
    while(1){
        sleep(10);
	    //printf("queue full = %d \b", *queue_full);
         // Check if queue 1 has data
         if(*queue_full != 1){
            //printf("queue 1 empty\n");
            queue_full++;
         }  
         else{
            // queue1 has data
            int min_fifo_prio = 32767, min_fifo_index = 32767;
            // Run through the queue to find which request to service
            for(i=0; i<requestsPerQueue; i++){
                // Assuming a max of 5 requests in each queue
                if(q1[i].fifo_priority < min_fifo_prio && q1[i].valid==0 && q1[i].full==1){
                    min_fifo_prio = q1[i].fifo_priority;
                    min_fifo_index = i;
                }
            }

            // min_fifo_index has the request with least FIFO priority, service it now
            if(min_fifo_index != 32767){
                printf("\n Servicing request from queue 1 at index %d \n",min_fifo_index);
                q1[min_fifo_index].output = call_service(q1[min_fifo_index].input);
                q1[min_fifo_index].valid = 1; // Indicate the client that the service is complete
            }  

         }
            
         // Checking queue2 is empty or not
         if(*queue_full != 1){
            printf("queue 2 empty\n");
            queue_full++;
         }
         else{
            // queue2 has data
            // Service 2 requests from queue2
            for(numRequest=0; numRequest<2; numRequest++){

                int min_fifo_prio = 32767, min_fifo_index = 32767;
                
                // Iterate over the queue to find which request to service
                for(i=0; i<requestsPerQueue; i++){
                    // Assuming a max of 5 requests in each queue
                    if(q2[i].fifo_priority < min_fifo_prio && q2[i].valid==0 && q2[i].full==1){
                        min_fifo_prio = q2[i].fifo_priority;
                        min_fifo_index = i;
                    }
                }

                // min_fifo_index has the request with least FIFO priority, service it now
                if(min_fifo_index != 32767){
                    printf("\n Servicing request from queue 2 at index %d \n",min_fifo_index);
                    q2[min_fifo_index].output = call_service(q2[min_fifo_index].input);
                    q2[min_fifo_index].valid = 1; // Indicate the client that the service is complete
                }    
            }    
         }

         // Checking queue3 is empty or not
         if(*queue_full != 1)
            queue_full++;
         else{
            // queue3 has data
            // Service 3 requests from queue3
            for(numRequest=0; numRequest<3; numRequest++){

                int min_fifo_prio = 32767, min_fifo_index = 32767;
                
                // Iterate over the queue to find which request to service
                for(i=0; i<requestsPerQueue; i++){
                    // Assuming a max of 5 requests in each queue
                    if(q3[i].fifo_priority < min_fifo_prio && q3[i].valid==0 && q3[i].full==1){
                        min_fifo_prio = q3[i].fifo_priority;
                        min_fifo_index = i;
                    }
                }

                // min_fifo_index has the request with least FIFO priority, service it now
                if(min_fifo_index != 32767){
                    printf("\n Servicing request from queue 3 at index %d \n",min_fifo_index);
                    q3[min_fifo_index].output = call_service(q3[min_fifo_index].input);
                    q3[min_fifo_index].valid = 1; // Indicate the client that the service is complete
                }    
            }    
         }


         // Checking queue4 is empty or not
         if(*queue_full != 1)
            queue_full-=3; // Move pointer back to queue1's flag
         else{
            // queue4 has data
            // Service 4 requests from queue4
            for(numRequest=0; numRequest<4; numRequest++){

                int min_fifo_prio = 32767, min_fifo_index = 32767;
                
                // Iterate over the queue to find which request to service
                for(i=0; i<requestsPerQueue; i++){
                    // Assuming a max of 5 requests in each queue
                    if(q4[i].fifo_priority < min_fifo_prio && q4[i].valid==0 && q4[i].full==1){
                        min_fifo_prio = q4[i].fifo_priority;
                        min_fifo_index = i;
                    }
                }

                // min_fifo_index has the request with least FIFO priority, service it now
                if(min_fifo_index != 32767){
                    printf("\n Servicing request from queue 4 at index %d \n",min_fifo_index);
                    q4[min_fifo_index].output = call_service(q4[min_fifo_index].input);
                    q4[min_fifo_index].valid = 1; // Indicate the client that the service is complete
                }    
            }    
         }
    }

   
    // if (q1->full == 1){
    //     printf("\n PID = %d, reqID= %d, input = %d, output= %d, fifo_priority = %d, valid= %d, full= %d \n", q1->PID, q1->reqID, q1->input, q1->output, q1->fifo_priority, q1->valid, q1->full);
    // }   

    // Detach and remove the shared memory segment
    
}

int call_service(int input){
    // Generate a random number
    return (rand()%100);
}
