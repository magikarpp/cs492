#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){

  if(argc != 6){
    printf("\nIncorrect Input.\n");
    return -1;
  }

  char* plist = argv[1];  //set plist path
  char* ptrace = argv[2]; //set ptrace path
  int pageSize = atoi(argv[3]);
  //check for correct pageSize: lessthan 0, power of 2, max page.
  //***
  char* replacementAlgorithm = argv[4]; //replacement Algorithm Method
  if (strcmp(replacementAlgorithm, "FIFO") && strcmp(replacementAlgorithm, "LRU") && strcmp(replacementAlgorithm, "Clock")){
    printf("\nIncorrect replacementAlgorithm: (options) FIFO, LRU, or Clock\n");
    return -1;
  }
  int prePaging;  //prePaging initialize
  if(!strcmp(argv[5], "+")){
    prePaging = 1;
  }else if(!strcmp(argv[5], "-")){
    prePaging = 0;
  }else{
    printf("\nIncorrect prePaging: (options) + or -\n");
  }

  printf("\nThis is working~ ...so far!\n");

  return 0;
}
