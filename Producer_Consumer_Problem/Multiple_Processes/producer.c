/*
* CSCI5103 Spring 2018
* Assignment# 4 Problem 2
* Scott Bossard
* ID: 4311794
* x500: boss0084

Producer.c is the code for the producer process which is a child created from main.c in the directory
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
int color; //used to distinguish between threads (colors)
int shid; // id for shared memory
struct shm *sharedMemPtr; // pointer to shared mem segment

/*************************/
/*Producer black code to be run by thread created in main()*/
/*************************/
void * producer_black (void *arg){
	//Create file to store data
  FILE *f = fopen("Producer_BLACK.txt", "w"); //open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
  int i; //for loop int
  //Producer produces 1000 items
  for (i = 0; i < 1000; i++) {
      pthread_mutex_lock (&(sharedMemPtr->lock));  //Enter critical section
      while (sharedMemPtr->count == sharedMemPtr->N)  // Check if buffer is full
          while (pthread_cond_wait(&(sharedMemPtr->SpaceAvailable), &(sharedMemPtr->lock)) != 0); //Wait until space available

      timeSys1 = gettimeofday(&t1Sys, NULL); // Get time
      char timeString[20]; //string to store time
      int currentTime = (t1Sys.tv_sec + t1Sys.tv_usec); //convert time
      sprintf(timeString, "BLACK %d\n", currentTime); //Put time in a string
      sharedMemPtr->Buffer[sharedMemPtr->in] = i; // Put item in the buffer
      sharedMemPtr->in = (sharedMemPtr->in + 1) % sharedMemPtr->N; // new value of in
      sharedMemPtr->count++; //Increment the count of items in the buffer
      printf ("%s\n",timeString);
      fprintf(f, "%s\n", timeString); //Print to file
      pthread_mutex_unlock (&(sharedMemPtr->lock)); //Unlock mutex
      pthread_cond_signal( &(sharedMemPtr->ItemAvailable)); //An item is now available so signal
   }
  sharedMemPtr->completed += 1; // Producer has completed so increment the number of producers completed
  fclose(f); //Close Producer_BLACK.txt"
}

/*************************/
/*Producer red code to be run by thread created in main()*/
/*************************/
void * producer_red(void *arg){
	//Create file to store data
  FILE *f = fopen("Producer_RED.txt", "w"); //open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
  int i; //for loop int
  //Producer produces 1000 items
  for (i = 0; i < 1000; i++) {
      pthread_mutex_lock (&(sharedMemPtr->lock));  //Enter critical section
      while (sharedMemPtr->count == sharedMemPtr->N)  // Check if buffer is full
          while (pthread_cond_wait(&(sharedMemPtr->SpaceAvailable), &(sharedMemPtr->lock)) != 0); //Wait until space available

      timeSys1 = gettimeofday(&t1Sys, NULL); // Get time
      char timeString[20]; //string to store time
      int currentTime = (t1Sys.tv_sec + t1Sys.tv_usec); //convert time
      sprintf(timeString, "RED %d\n", currentTime); //Put time in a string
      sharedMemPtr->Buffer[sharedMemPtr->in] = i; // Put item in the buffer
      sharedMemPtr->in = (sharedMemPtr->in + 1) % sharedMemPtr->N; // new value of in
      sharedMemPtr->count++; //Increment the count of items in the buffer
      printf ("%s\n",timeString);
      fprintf(f, "%s\n", timeString); //Print to file
      pthread_mutex_unlock (&(sharedMemPtr->lock)); //Unlock mutex
      pthread_cond_signal( &(sharedMemPtr->ItemAvailable)); //An item is now available so signal
   }
  sharedMemPtr->completed += 1; // Producer has completed so increment the number of producers completed
  fclose(f); //Close Producer_BLACK.txt"
}

/*************************/
/*Producer white code to be run by thread created in main()*/
/*************************/
void * producer_white(void *arg){
	//Create file to store data
  FILE *f = fopen("Producer_WHITE.txt", "w"); //open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
  int i; //for loop int
  //Producer produces 1000 items
  for (i = 0; i < 1000; i++) {
      pthread_mutex_lock (&(sharedMemPtr->lock));  //Enter critical section
      while (sharedMemPtr->count == sharedMemPtr->N)  // Check if buffer is full
          while (pthread_cond_wait(&(sharedMemPtr->SpaceAvailable), &(sharedMemPtr->lock)) != 0); //Wait until space available

      timeSys1 = gettimeofday(&t1Sys, NULL); // Get time
      char timeString[20]; //string to store time
      int currentTime = (t1Sys.tv_sec + t1Sys.tv_usec); //convert time
      sprintf(timeString, "WHITE %d\n", currentTime); //Put time in a string
      sharedMemPtr->Buffer[sharedMemPtr->in] = i; // Put item in the buffer
      sharedMemPtr->in = (sharedMemPtr->in + 1) % sharedMemPtr->N; // new value of in
      sharedMemPtr->count++; //Increment the count of items in the buffer
      printf ("%s\n",timeString);
      fprintf(f, "%s\n", timeString); //Print to file
      pthread_mutex_unlock (&(sharedMemPtr->lock)); //Unlock mutex
      pthread_cond_signal( &(sharedMemPtr->ItemAvailable)); //An item is now available so signal
   }
  sharedMemPtr->completed += 1; // Producer has completed so increment the number of producers completed
  fclose(f); //Close Producer_BLACK.txt"
}

int main (int argc, char *argv[]){
  shid = shmget (atoi(argv[1]), 0, 0); //use shmget and set to shared meme id. argv[1] = shared mem key
  if (shid == -1)
  {
		printf("Error in shmget (producer)");
		exit (1);
  }

  sharedMemPtr = shmat (shid, (void *) NULL, sizeof(struct shm));//use shamt attach to shared memeory pointer
  if (sharedMemPtr == (void *) -1)
  {
      printf("Error in shmat (producer)");
      exit (1);
  }

  color = atoi(argv[2]); //color argument 0 = black, 1 =red, 2 = white
  pthread_t black, red, white; //thread variables
  pthread_attr_t attr;  //attribute object
  pthread_attr_init(&attr);

	//Create producer threads
	//If color = 0 then start black producer thread
  int b;
  int r;
  int w;
  printf("Color %d arrived\n", color);
  switch(color){
    case 0:
		if (b = pthread_create(&black, &attr, producer_black, NULL)) {
				 fprintf(stderr,"pthread_create (black):%s\n",strerror(b));
				 exit(1);
		}
    if (b = pthread_join(black, NULL)) {
         fprintf(stderr,"pthread_join:%s\n",strerror(b));
         exit(1);
    }
    break;

	//If color = 1 then start red producer thread
  case 1:
		if (r = pthread_create(&red, &attr, producer_red, NULL)) {
				 fprintf(stderr,"pthread_create (red):%s\n",strerror(r));
				 exit(1);
		}
    if (r = pthread_join(red, NULL)) {
         fprintf(stderr,"pthread_join:%s\n",strerror(r));
         exit(1);
    }
	  break;

	//If color = 2 then start white producer thread
	case 2:
		if (w = pthread_create(&white, &attr, producer_white, NULL)) {
				 fprintf(stderr,"pthread_create (white):%s\n",strerror(w));
				 exit(1);
		}
    if (w = pthread_join(white, NULL)) {
         fprintf(stderr,"pthread_join:%s\n",strerror(w));
         exit(1);
    }
    break;
  default:
      return 0;
	}
  // Detach shared memory
	if((shmdt((void *)sharedMemPtr)) == -1){
			printf("Deattaching failed (producer)\n");
			exit(1);
	}
}
