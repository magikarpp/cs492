#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>

int createdProducts = 0;
int consumedProducts = 0;
int maxProducts = 0;
int productsQueued = 0;
int queueSize = 0;
int end = -1;
int front = 0;
int quantumVal;
int schedulingVal;

pthread_mutex_t mainMutex;
pthread_cond_t producerCondition;
pthread_cond_t consumerCondition;

//struture for product.
struct product{
  int product_id;
  double time_stamp;
  int life;
};

//create the product queue as an array of products.
struct product *productQueue;

//check if element is an integer
int isInt(char number[]){
  for(int i = 0; number[i] != 0; i++){
    if(!isdigit(number[i])){
      return 0;
    }
  }
  return 1;
}

//get the time.
double getTime(void){
  struct timeval tv;
  double timeval = tv.tv_sec;
  return timeval;
}

//insert a product into the queue.
void insert(struct product newProduct){
  if(productsQueued != queueSize){
    if(end == queueSize - 1){
      end = -1;
    }
    productQueue[++end] = newProduct;
    productsQueued++;
  }
}

//remove a product from the queue.
struct product removeFromQueue(){
  struct product removed_product = productQueue[front++];
  if(front == queueSize){
    front = 0;
  }
  productsQueued--;
  return removed_product;
}

//fibonacci function.
int fn(int n){
  if (n <= 1){
    return n;
  }

  return fn(n-1) + fn(n-2);
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
    if(scheduling != 1){
      quantum = 1025;
    }
    quantumVal = quantum;
    schedulingVal = scheduling;

    int producerID[producer_number];
    int consumerID[consumer_number];
    pthread_t producer_thread[producer_number];
    pthread_t consumer_thread[consumer_number];
    void *producer();
    void *consumer();
    maxProducts = total_products;
    productQueue = malloc(sizeof(struct product)*queue_size);
    queueSize = queue_size;

    //initialize mutex & condition variables.
    pthread_mutex_init(&mainMutex, NULL);
    pthread_cond_init(&consumerCondition, NULL);
    pthread_cond_init(&producerCondition, NULL);

    srandom(seed);

    //create threads
    for(int i = 0; i < producer_number; i++){
      producerID[i] = i;
      pthread_create(&producer_thread[i], NULL, producer, &producerID[i]);
    }
    for(int i = 0; i< consumer_number; i++){
      consumerID[i] = i;
      pthread_create(&consumer_thread[i], NULL, consumer, &consumerID[i]);
    }

    //join threads
    for(int i = 0; i < producer_number; i++){
      pthread_join(producer_thread[i], NULL);
    }
    for(int i = 0; i < consumer_number; i++){
      pthread_join(consumer_thread[i], NULL);
    }

    free(productQueue);

    printf("\nThis is working!\n");
    return;

  }else{
      printf("All 8 arguements must be provided.\n");
      return;
  }
}

//adds products to the queue and finishes when it reaches the given maximum number of products.
void *producer(int *thread){
  while(createdProducts < maxProducts){
    //lock mutex.
    pthread_mutex_lock(&mainMutex);
    //if queue is full, wait.
    while(createdProducts < maxProducts && productsQueued >= queueSize){
      pthread_cond_wait(&producerCondition, &mainMutex);
    }
    //add product to the queue.
    struct product created_product;
    created_product.product_id = createdProducts;
    created_product.time_stamp = getTime();
    created_product.life = random()%1024;
    insert(created_product);
    createdProducts++;
    printf("Producer %d has produced product %d.\n", *thread, created_product.product_id);

    //signals consumer thread and unlock mutex.
    pthread_cond_broadcast(&consumerCondition);
    pthread_mutex_unlock(&mainMutex);
    //sleep for 100milliseconds.
    usleep(100000);
  }
  pthread_exit(0);
}

//consumes products from the queue and finishes when max consumption number is reached.
void *consumer(int *thread){
  struct product consumed_product;
  double take_time = 0;
  double put_time = 0;
  double turn_around = 0;
  while(consumedProducts < maxProducts){
    pthread_mutex_lock(&mainMutex);
    while(createdProducts < maxProducts && productsQueued < queueSize){
      pthread_cond_wait(&consumerCondition, &mainMutex);
    }
    //if there are products in the queue, remove and return its value to consumed_product.
    if(productsQueued != 0){
      consumed_product = removeFromQueue();
    }
    //Round Robin: if quantum value is less than life of product, we are able to subtract it from the product's life.
    if(consumed_product.life > quantumVal && schedulingVal == 1){
      //reduce life of consumed_product by the quantum value.
      consumed_product.life = consumed_product.life - quantumVal;
      for(int i = quantumVal; i > 0; i--){
        fn(10);
      }
      //return consumed_product back into the queue with its reduced life.
      insert(consumed_product);
    } else{ //First Come First Serve.
      for(int i = consumed_product.life; i > 0; i--){
        fn(10);
      }
      consumedProducts++;
      printf("Consumer %d has consumed product %d.\n", *thread, consumed_product.product_id);
    }
    pthread_cond_broadcast(&producerCondition);
    pthread_mutex_unlock(&mainMutex);
    usleep(100000);
  }
  pthread_exit(0);
}
