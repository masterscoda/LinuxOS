/*
* CSCI5103 Spring 2018
* Assignment# 4
* Scott Bossard
* ID: 4311794
* x500: boss0084

To run program:

  1. Open terminal and navigate to the directory of the file
  2. Type 'make' in the terminal
  3. type './prod_cons' in terminal
  4. Open consumer.txt, producer_black.txt, producer_red.txt and producer_white.txt to view outputs


Each producer process randomly creates a specific colored item: producer_red creates RED colored items,
producer_black creates BLACK colored items, and producer_white creates WHITE colored items. Associated
with each item created by a specific producer is a local LINUX timestamp in microseconds which records the time that item was placed into the buffer. Each producer deposits its
item into the buffer, and the consumer retrieves the items from the buffer. The items produced by the producers
will be of any of the three strings: “RED <timestamp>”, “BLACK <timestamp>”, “WHITE <timestamp>”
Each producer process, after successfully depositing an item into the buffer, will write that item (essentially the
“COLOR timestamp” string) into its log file called Producer_COLOR.txt (e.g. Producer_RED.txt). The consumer
process will then retrieve the items from this shared buffer. When it retrieves an item, the consumer process will
write that item (essentially the “COLOR timestamp” string) into a file called Consumer.txt.
*/


#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

const int N = 2; // For keeping track of Buffer availability
char *Buffer[2]; // Maximum of 2 items in Buffer
char *(*ptr)[2] = &Buffer; //pointer the the buffer to help print
int in = 0; //buffer element for producer to insert
int out = 0; //buffer element for consumer to take out
int count = 0;
int completed = 0; //number of threads completed. 3 = completed with all 3 producers
pthread_mutex_t  lock; //mutex lock for critical section
//condition variables that allow producer to produce if space available and consumer to consume if item available
pthread_cond_t  SpaceAvailable, ItemAvailable;
int timeSys1; // Used to store gettimeofday()
struct timeval t1Sys; // Time struct

/*************************/
/*Producer of Black color*/
/*************************/
void * producer_black (void *arg){
  FILE *f = fopen("Producer_BLACK.txt", "w"); //open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
  int color;
  color = *(int *)arg; //color argument (used for debuging)
  int i; //for loop int
  //Producer produces 1000 items
  for ( i = 0; i < 1000; i++) {
      pthread_mutex_lock (&lock);  //Enter critical section
      while (count == N)  // Check if buffer is full
          while (pthread_cond_wait(&SpaceAvailable, &lock) != 0); //Wait until space available

      timeSys1 = gettimeofday(&t1Sys, NULL); // Get time
      char timeString[20]; //string to store time
      int currentTime = (t1Sys.tv_sec + t1Sys.tv_usec); //convert time
      sprintf(timeString, "BLACK %d\n", currentTime); //Put time in a string
      (*ptr)[in] = timeString; // Put item in the buffer using ptr and in
      in = (in + 1) % N; // new value of in
      count++; //Increment the count of items in the buffer
      fprintf(f, "%s\n", timeString); //Print to file
      pthread_mutex_unlock ( &lock); //Unlock mutex
      pthread_cond_signal( &ItemAvailable ); //An item is now available so signal
   }
  completed += 1; // Producer has completed so increment the number of producers completed
  fclose(f); //Close Producer_BLACK.txt"
}

/*************************/
/*Producer of Red color*/
/*************************/
void * producer_red (void *arg){
  FILE *f = fopen("Producer_RED.txt", "w");//open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
  int color;
  color = *(int *)arg; //color argument (used for debuging)
  int i; //for loop int
  //Producer produces 1000 items
  for ( i = 0; i < 1000; i++) {
      pthread_mutex_lock ( &lock);  //Enter critical section
      while ( count == N ) // Check if buffer is full
          while (pthread_cond_wait( &SpaceAvailable, &lock) != 0); //Wait until space available

      timeSys1 = gettimeofday(&t1Sys, NULL); // Get time
      char timeString[20]; //string to store time
      int currentTime = (t1Sys.tv_sec + t1Sys.tv_usec); //convert time
      sprintf(timeString, "RED %d\n", currentTime); //Put time in a string
      (*ptr)[in] = timeString; // Put item in the buffer using ptr and in
      in = (in + 1) % N; // new value of in
      count++; //Increment the count of items in the buffer
      fprintf(f, "%s\n", timeString); //Print to file
      pthread_mutex_unlock (&lock); //Unlock mutex
      pthread_cond_signal(&ItemAvailable); //An item is now available so signal
   }
   completed += 1; // Producer has completed so increment the number of producers completed
   fclose(f);//Close Producer_RED.txt"
}

/*************************/
/*Producer of White color*/
/*************************/
void * producer_white (void *arg){
  FILE *f = fopen("Producer_WHITE.txt", "w");//open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
  int color;
  color = *(int *)arg; //color argument (used for debuging)
  int i; //for loop int
  //Producer produces 1000 items
  for ( i = 0; i < 1000; i++) {
      pthread_mutex_lock ( &lock);  //Enter critical section
      while ( count == N ) // Check if buffer is full
          while (pthread_cond_wait( &SpaceAvailable, &lock) != 0); //Wait until space available

      timeSys1 = gettimeofday(&t1Sys, NULL); // Get time
      char timeString[20];//string to store time
      int currentTime = (t1Sys.tv_sec + t1Sys.tv_usec); //convert time
      sprintf(timeString, "WHITE %d\n", currentTime); //Put time in a string
      (*ptr)[in] = timeString; // Put item in the buffer using ptr and in
      in = (in + 1) % N; // new value of in
      count++; //Increment the count of items in the buffer
      fprintf(f, "%s\n", timeString); //Print to file
      pthread_mutex_unlock (&lock); //Unlock mutex
      pthread_cond_signal(&ItemAvailable); //An item is now available so signal
   }
   completed += 1; // Producer has completed so increment the number of producers completed
   fclose(f); //Close Producer_WHITE.txt"
}

/*************************/
/*Consumer of colors*/
/*************************/
void * consumer (void *arg){
  FILE *f = fopen("consumer.txt", "w"); //open file or create new if not existing
  //error check
  if (f == NULL)
  {
    printf("Error opening file!\n");
    exit(1);
  }
  do {
    pthread_mutex_lock ( &lock);  //Enter critical section
    while (count == 0)  // Check if buffer is empty
        while (pthread_cond_wait( &ItemAvailable, &lock) != 0) ; //Wait until item available
    //if buffer isnt empty then consume
    if  (count > 0) {
          fprintf(f, "Removed: %s\n", (*ptr)[out]); //print item removed to file
          out = (out + 1) % N; //new value of out
          count--; //Decrement the count of items in the buffer
    }
    pthread_mutex_unlock ( &lock);   //Unlock mutex
    pthread_cond_signal( &SpaceAvailable );  //Space is now available so signal
  } while ( completed != 3 ); //do until all 3 producers are completed
  fclose(f); //close consumer.txt
}

/*************************/
/*Main*/
/*************************/
int main( int argc, char* argv[] ){
  pthread_t prod1, prod2, prod3, cons; //thread variables
  pthread_attr_t attr; //attribute object
  int n; //int used to create thread
  int black = 0; //black producer id
  int red = 1;//red producer id
  int white = 2;//white producer id
  pthread_mutex_init( &lock, NULL); //initialize mutex
  pthread_cond_init( &SpaceAvailable, NULL); //initialize condition
  pthread_cond_init( &ItemAvailable, NULL);//initialize condition
  //Create black producer thread
  if (n = pthread_create(&prod1, NULL, producer_black, &black)) {
       fprintf(stderr,"pthread_create :%s\n",strerror(n));
       exit(1);
  }
  //Create red producer thread
  if (n = pthread_create(&prod2, NULL, producer_red, &red)) {
       fprintf(stderr,"pthread_create :%s\n",strerror(n));
       exit(1);
  }
  //Create white producer thread
  if (n = pthread_create(&prod2, NULL, producer_white, &white)) {
     fprintf(stderr,"pthread_create :%s\n",strerror(n));
     exit(1);
  }
  //Create consumer thread
  if (n = pthread_create(&cons, NULL, consumer, NULL)) {
        fprintf(stderr,"pthread_create :%s\n",strerror(n));
        exit(1);
  }
  //Wait for the consumer thread to finish.
  if (n = pthread_join(cons, NULL)) {
       fprintf(stderr,"pthread_join:%s\n",strerror(n));
       exit(1);
  }
  printf("Finished execution \n" ); //All done
}
