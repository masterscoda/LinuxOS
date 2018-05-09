/*
* CSCI5103 Spring 2018
* Assignment #7
* Scott Bossard
* ID: 4311794
* x500: boss0084
* Tested on Virtual Machine: csel-x12-umh.cselabs.umn.edu

producer.c is the code for the producer process
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv){
  int f; //Used for file to store data
  //Error check (-1 if a error)
  if((f = open("/dev/scullbuffer", O_WRONLY)) == -1){
    printf("Error while opening file (Producer.c)\n");
    return 0;
  }

  int numOfItems; //to store user input for number of items to produce
  numOfItems = atoi(argv[1]); //convert the user input for # of items to consume to integer
  char *color; //to store user input for color of item to produce
  color = argv[2]; //assign user input of color
  char buffer[32]; //create buffer. 32B is item size
  int i;
  for(i = 0; i < numOfItems; i++){
    sprintf(buffer, "%s%08d", color, i); //Put buffer item in string. Limit string length to 8
    printf("Producing item %s\n", buffer);
    if(write(f, buffer, 32) == 0){
      //if 0 then there is nothing to write
      printf("Nothing to write\n");
      return 0;
    }
    usleep(100); //Sleep 100 microseconds
  }
  close(f);
}
