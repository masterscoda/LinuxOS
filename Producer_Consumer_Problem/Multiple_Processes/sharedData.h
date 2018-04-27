/*
* CSCI5103 Spring 2018
* Assignment# 4 Problem 2
* Scott Bossard
* ID: 4311794
* x500: boss0084

Header file that contains the data structure shared between process
*/

#include <pthread.h>

struct shm{
  int N; // For keeping track of Buffer availability
  int Buffer[2]; // Maximum of 2 items in Buffer
  int in; //buffer element for producer to insert
  int out; //buffer element for consumer to take out
  int count; //count integer to help to see if buffer is full
  int completed; //number of threads completed. 3 = completed with all 3 producers
  pthread_mutex_t  lock; //mutex lock for critical section
  //condition variables that allow producer to produce if space available and consumer to consume if item available
  pthread_cond_t  SpaceAvailable;
  pthread_cond_t ItemAvailable;
};
