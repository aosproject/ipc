#include "common.h"
#include <time.h>

#define SHMSZ1 800 // 800 bytes of shared memory for 4 priority queues
#define SHMSZ2 100  // 100 bytes of shared memory for 4 flags per queue to denote full/empty queue

int call_service(int);

main()
{
    char c;
    int shmid1, shmid2;
    key_t key1, key2;
    struct client_request *q1,*q2,*q3,*q4; // Four priority queues
    int *shm_full;
    int requestsPerQueue = 5, numRequest, i;
    srand(time(NULL)); 

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

    if ((shm_full = (int*)shmat(shmid2, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    // 40 bytes for 1 request
    // printf("size of structure object = %lu\n", sizeof(client_request));


    // q2, q3, q4 point to head of individual queues
    q2 = q1 + requestsPerQueue; 
    q3 = q2 + requestsPerQueue;
    q4 = q3 + requestsPerQueue;

    // Loop forever
    while(1){

         // Check if queue 1 has data
         if(*shm_full != 1)
            shm_full++;
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
                q1[min_fifo_index].output = call_service(q1[min_fifo_index].input);
                q1[min_fifo_index].valid = 1; // Indicate the client that the service is complete
            }  

         }
            
         // Checking queue2 is empty or not
         if(*shm_full != 1)
            shm_full++;
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
                    q2[min_fifo_index].output = call_service(q2[min_fifo_index].input);
                    q2[min_fifo_index].valid = 1; // Indicate the client that the service is complete
                }    
            }    
         }

         // Checking queue3 is empty or not
         if(*shm_full != 1)
            shm_full++;
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
                    q3[min_fifo_index].output = call_service(q3[min_fifo_index].input);
                    q3[min_fifo_index].valid = 1; // Indicate the client that the service is complete
                }    
            }    
         }


         // Checking queue4 is empty or not
         if(*shm_full != 1)
            shm_full-=3; // Move pointer back to queue1's flag
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
    shmdt(q1);
    shmdt(shm_full);

    shmctl(shmid1, IPC_RMID, NULL);
    shmctl(shmid2, IPC_RMID, NULL);

    exit(0);
}

int call_service(int input){
    // Generate a random number
    return (rand()%100);
}