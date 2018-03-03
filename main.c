#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>

struct product{
  int product_id;
  double time_stamp;
  int life;
};

//function: check if element if a number.
int isInt(char number[]){
  for(int i = 0; number[i] != 0; i++){
    if(!isdigit(number[i])){
      return 0;
    }
  }
  return 1;
}

void main(int argc, char** argv){
  //check if all 8 arguements are provided.
  if(argc == 8){
    for(int i = 1; i < 8; i++){
      //arguement must be an integer and not empty.
      if(!isInt(argv[i]) && argv[i] != NULL){
        printf("Input arguement #%d is not a number.\n", i);
        return;
      }
    }

    int producer_number = atoi(argv[1]);  //P1: number of producer threads
    int consumer_number = atoi(argv[2]);  //P2: number of consumer threads
    int total_products = atoi(argv[3]);   //P3: total number of products to be generated
    int queue_size = atoi(argv[4]);       //P4: size of queue
    int scheduling = atoi(argv[5]);       //P5: 0 = FCFS, 1 = Round Robin
    int quantum = atoi(argv[6]);          //P6: value of quantum used for round-robin shceduling
    int seed = atoi(argv[7]);             //P7: seed for random number generator

  }else{
      printf("All 8 arguements must be provided.\n");
      return;
  }
}
