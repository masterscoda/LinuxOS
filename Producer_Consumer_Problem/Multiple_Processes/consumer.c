/*
* CSCI5103 Spring 2018
* Assignment# 4 Problem 2
* Scott Bossard
* ID: 4311794
* x500: boss0084

Conumer.c is the code for the consumer process which is a child of main.c in the directory
*/

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include "sharedData.h" //contains data structure to be shared across processes

//Initialize global variables
int timeSys1; // Used to store gettimeofday()
struct timeval t1Sys; // Time struct
int shid; // id for shared memory
struct shm *sharedMemPtr; // pointer to shared mem segment

/*************************/
/*Consumer code to be run by thread created in main()*/
/*************************/
void * consumer_all (void *arg){
	//Create file to store data
  FILE *f = fopen("consumer.txt", "w"); //open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
	int i = 0; // store buffer data
  do {
    pthread_mutex_lock ( &(sharedMemPtr->lock));  //Enter critical section
    while (sharedMemPtr->count == 0)  // Check if buffer is empty
        while (pthread_cond_wait( &(sharedMemPtr->ItemAvailable), &(sharedMemPtr->lock)) != 0) ; //Wait until item available
    //if buffer isnt empty then consume
    if  (sharedMemPtr->count > 0) {
    	i = sharedMemPtr->Buffer[sharedMemPtr->out]; //Get data from buffer
      fprintf (f,"Removed %d \n", i); //Print to file
      printf ("Removed %d \n", i); //Print to terminal
      sharedMemPtr->out = (sharedMemPtr->out + 1) % sharedMemPtr->N; //new value of out
      sharedMemPtr->count--; //Decrement the count of items in the buffer
    }
    pthread_mutex_unlock (&(sharedMemPtr->lock));   //Unlock mutex
    pthread_cond_signal( &(sharedMemPtr->SpaceAvailable));  //Space is now available so signal
  } while ( sharedMemPtr->completed != 3 ); //do until all 3 producers are completed
  fclose(f); //close consumer.txt
}

/*************************/
/*Main Consumer code*/
/*************************/
int main (int argc, char *argv[]){
  shid = shmget (atoi(argv[1]), 0, 0); //use shmget and set to shared meme id. argv[1] = shared mem key
  if (shid == -1) //error check
  {
      printf("Error in shmget (consumer)");
      exit (1);
  }

  sharedMemPtr = shmat (shid, (void *) NULL, sizeof(struct shm)); //use shamt attach to shared memeory pointer
  if (sharedMemPtr == (void *) -1) //error check
  {
      printf ("Error in shmat (consumer)");
      exit (1);
  }

	//initialize variables
  pthread_t cons; //thread variables
  pthread_attr_t attr; //attribute object
  pthread_attr_init(&attr); //initialize attribute

	//Create consumer thread
  int n; //int used to create thread
  if (n = pthread_create(&cons, &attr, consumer_all, NULL) ) {
  	fprintf(stderr,"pthread_create consumer:%s\n",strerror(n));
    exit(1);
  }
  //Wait for the consumer thread to finish.
  if (n = pthread_join(cons, NULL) ) {
    fprintf(stderr,"pthread_join consumer:%s\n",strerror(n));
    exit(1);
  }
	//Detach memory
  if((shmdt((void *)sharedMemPtr)) == -1){
			printf("Deattaching failed (consumer)\n");
			exit(2);
	}
}
