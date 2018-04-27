/*
* CSCI5103 Spring 2018
* Assignment# 4 Problem 2
* Scott Bossard
* ID: 4311794
* x500: boss0084


To run program:

  1. Open terminal and navigate to the directory of the file
  2. Type 'make' in the terminal
  3. type './main' in terminal
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

The main program, in a file named main.c, will fork these four processes. One of these will execute the
consumer process code (source for which will be in a file named consumer.c), and the three will execute the
producer process code (source for which will be in a file named producer.c). While forking the producer
processes, make sure that the main function passes the “color” as a parameter to each of the producer
processes, so that each of the three producer processes knows what colored items it will create. The main
program will also create a shared memory segment between these four processes. After creating these
processes, it will wait for all of them to terminate.
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


/*************************/
/*Main*/
/*************************/
int main( int argc, char* argv[] ){
  //Initialize variables
  pid_t childCon, childBlack, childRed, childWhite; //child process id
  int childConsReturn, childBlackReturn, childRedReturn, childWhiteReturn, childConsStatus, childBlackStatus, childRedStatus, childWhiteStatus; //used to check if child process is done
  int shmemID; // id for shared memory
  struct shm *shmemPtr; // pointer to shared mem segment
  key_t key; // A key to access shared memory segments
  int flag; // for permissions

  //Set variables
  key = 6; // Some arbitrary integer, which will also be passed to the other processes which need to share memory
  flag = 1023;  // 1023 = 111111111 in binary, i.e. all permissionsand modes are set.
  shmemID = shmget (key, sizeof(struct shm), flag); //use shmget and set to shared meme id
  if (shmemID == -1) //error check
  {
      printf("Error in shmget");
      exit (1);
  }
  shmemPtr = shmat (shmemID, (void *)NULL , sizeof(struct shm)); //use shamt to attach to shared memeory pointer
  if (shmemPtr == (void *) -1) //error check
  {
      printf ("Error in shmat");
      exit (1);
  }
  //Initialize shared memory data
  shmemPtr->N = 2; //number of elements in buffer
  shmemPtr->in = 0; //buffer element for producer to insert
  shmemPtr->out = 0; //buffer element for consumer to take out
  shmemPtr->count = 0; //keep track of buffer size
  shmemPtr->completed = 0; //number of threads completed. 3 = completed with all 3 producers
  pthread_mutexattr_t mattr; //mutex attribute object
  pthread_condattr_t cattr; //mutex attribute object
  pthread_mutexattr_init(&mattr); //initialize attribute
  pthread_condattr_init(&cattr); //initialize attribute
  pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK_NP);
  pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED); //Set mutex to process shared so all can access
  pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED); //Set condition to process shared so all can access
  pthread_mutex_init( &shmemPtr->lock, &mattr); //initialize mutex
  pthread_cond_init( &shmemPtr->SpaceAvailable, &cattr); //initialize condition
  pthread_cond_init( &shmemPtr->ItemAvailable, &cattr);//initialize condition

  //Begin to Fork processes
  //First fork consumer process
  if((childCon = fork()) == -1){
    //error check
    printf("Fork error w/ Consumer :\n");
    exit(1);
  }
  if(childCon == 0){
    char keystr[10];
    sprintf (keystr, "%d", key); //store key to pass as parameter
    execl("./consumer", "consumer", keystr, NULL); // create consumer child process
  }
  else{
    // Next create black producer process
    if((childBlack = fork()) == -1){
      //error check
      printf("Fork error w/ black producer\n");
      exit(1);
    }
    if(childBlack == 0){
      char keystr[10];
      sprintf (keystr, "%d", key);//store key to pass as parameter
      execl("./producer", "producer", keystr, "0", NULL); // create black producer process with parameter 0 = black producer id. Use 0 to distingush between producer in producer.c
    }
    else{
      // Next create red producer process
      if((childRed = fork()) == -1){
        //error check
        printf("Fork error w/ red producer\n");
        exit(1);
      }
      if(childRed == 0){
        char keystr[10];
        sprintf (keystr, "%d", key);//store key to pass as parameter
        execl("./producer", "producer", keystr, "1", NULL); // create red producer process with parameter 1 = red producer id. Use 1 to distingush between producer in producer.c
      }
      else{
        // Next create white producer process
        if((childWhite = fork()) == -1){
          //error check
          printf("Fork error w/ white producer\n");
          exit(1);
        }
        if(childWhite == 0){
          char keystr[10];
          sprintf (keystr, "%d", key);//store key to pass as parameter
          execl("./producer", "producer", keystr, "2", NULL); // create white producer process with parameter 2 = white producer id. Use 2 to distingush between producer in producer.c
        }
        else{
          //Done creating child process so all thats left is parent
          //Waiting for the child processes to finish
          //Consumer process will be the last one finished
          while( childBlackReturn != childBlack || childRedReturn != childRed || childWhiteReturn != childWhite || childConsReturn != childCon){
            childRedReturn = waitpid(childRed, &childRedStatus, 0);
            if (WIFEXITED(childRedStatus)) {
              if (childRedStatus == 0){
                printf("Red finished\n");
              }
              else{
                printf("Error in Red\n");
                exit(1);
              }
            }
            childBlackReturn = waitpid(childBlack, &childBlackStatus, 0);
            if (WIFEXITED(childBlackStatus)) {
              if (childBlackStatus == 0){
                printf("Black finished\n");
              }
              else{
                printf("Error in Black\n");
                exit(1);
              }
            }
            childWhiteReturn = waitpid(childWhite, &childWhiteStatus, 0);
            if (WIFEXITED(childWhiteStatus)) {
              if (childWhiteStatus == 0){
                printf("White finished\n");
              }
              else{
                printf("Error in White\n");
                exit(1);
              }
            }
            childConsReturn = waitpid(childCon, &childConsStatus, 0);
            if (WIFEXITED(childConsStatus)) {
              if (childConsStatus == 0){
                printf("Consumer finished\n");
              }
              else{
                printf("Error in consumer\n");
                exit(1);
              }
            }
          }
          // Detach shared memory
          if((shmdt((void*)shmemPtr)) == -1){
              printf("Error in deattaching\n");
              exit(1);
          }
          //Remove shared memory
          if((shmctl(shmemID, IPC_RMID, NULL)) == -1){
              printf("Error in removing \n");
              exit(1);
          }
          printf("Finished execution \n" ); //All done
        }
      }
    }
  }
}
