/*
* CSCI5103 Spring 2018
* Assignment #7
* Scott Bossard
* ID: 4311794
* x500: boss0084
* Tested on Virtual Machine: csel-x12-umh.cselabs.umn.edu

consumer.c is the code for the consumer process
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv){
  int f; //Used for file to store data
  //Error check (-1 if a error)
  if((f = open("/dev/scullbuffer", O_RDONLY)) == -1){
    printf("Error while opening file (Consumer.c) \n");
    return 0;
  }

  int numOfItems; //to store user input for number of items to consume
  numOfItems = atoi(argv[1]); //convert the user input for # of items to consume to integer
  char buffer[32]; //create buffer. 32B is item size
  int i;
  for(i = 0; i < numOfItems; i++){
    //Call read the number of times specified by user
    if(read(f, buffer, 32) == 0){
      //if 0 then there is nothing to read
      printf("Nothing to consume...\n");
      return 0;
    }
    else{
      //If not =0 then we can read the item
      printf("Consuming item %s\n", buffer);
    }
    usleep(100); //Sleep 100 microseconds
  }
  close(f);
}
