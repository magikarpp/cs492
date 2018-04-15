#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

struct page{
    int number;
    int validBit;
    unsigned long accessed;
};

struct fifo{
    int pageNumber;
    int repChance;
    struct fifo* next;
};

struct pageTable{
    struct page** pages;
    int numPages;
    int numLoaded;
    struct fifo* head;
};

long unsigned int pageSwapCount; //global page swap counter

//create pageTable
struct pageTable* createPageTable(int pSize, int pTotal){
    struct pageTable* ret;
    struct page* hol;
    struct page* p;
    int i;
    int numPages = (int)ceil((double)pTotal / pSize);

    ret = (struct pageTable*)malloc(sizeof(struct pageTable));
    if (ret != NULL){
        ret->pages = (struct page**)malloc(sizeof(struct page*)*numPages);
        if (ret->pages != NULL){
            ret->numPages = numPages;
            ret->numLoaded = 0;

            for (i = 0; i < numPages; i++){
                //create page for pageTable
                hol = (struct page*)malloc(sizeof(struct page));
                if (hol != NULL){
                    if (!pageSwapCount){
                    pageSwapCount = 0;
                  }
                  hol->number = ++pageSwapCount;
                  hol->validBit = 0;
                  hol->accessed = 0;
                }

                p = hol;
                if (p == NULL){
                    printf("Error: memory allocation for new page\n");
                    fflush(stdout);
                    break;
                }
                ret->pages[i] = p;
            }

            ret->head = NULL;
        }
    }
    return ret;
}

int popFifo(struct pageTable* tab)
{
    int ret;
    struct fifo* tmp;

    ret = -1;
    if (tab->head != NULL)
    {
        tmp = tab->head->next;

        ret = tab->head->pageNumber;
        free(tab->head);
        tab->head = tmp;
    }

    return ret;
}

void pushFifo(struct pageTable* tab, int num)
{
    struct fifo* tmp;
    struct fifo* n;

    n = (struct fifo*)malloc(sizeof(struct fifo));
    if (n != NULL)
    {
        n->pageNumber = num;
        n->next = NULL;

        if (tab->head == NULL)
            tab->head = n;
        else
        {
            tmp = tab->head;

            while (tmp->next != NULL)
                tmp = tmp->next;
            tmp->next = n;
        }
    }
    else
        printf("Error allocating memory with struct fifo\n");
}

int indexOfLRUValidPage(struct pageTable* tab)
{
    int i;
    int ret;
    unsigned long min;

    ret = -1;
    min = -1;
    for (i = 0; i < tab->numPages; i++)
    {
        if (!tab->pages[i]->validBit)
            continue;

        if (tab->pages[i]->accessed < min)
        {
            min = tab->pages[i]->accessed;
            ret = i;
        }
    }

    return ret;
}

int popClock(struct pageTable* tab)
{
    int ret;
    struct fifo* tmp;
    struct fifo* f;

    ret = -1;
    if (tab->head != NULL)
    {
        tmp = tab->head;

        while (1)
        {
            if (tmp->repChance) /* set repChance to zero and add to end */
            {
                tmp->repChance = 0;

                /* update in place if head has one element */
                if (tab->head->next != NULL)
                    tab->head = tmp->next;

                f = tmp;
                while (f->next != NULL)
                    f = f->next;
                f->next = tmp;
                tmp->next = NULL;

                /* start again at the new head */
                tmp = tab->head;
            }
            else /* return this one */
            {
                ret = tmp->pageNumber;
                tab->head = tmp->next;
                free(tmp);
                break;
            }
        }
    }

    return ret;
}

void pushClock(struct pageTable* tab, int num)
{
    struct fifo* tmp;
    struct fifo* n;

    n = (struct fifo*)malloc(sizeof(struct fifo));
    if (n != NULL)
    {
        n->pageNumber = num;
        n->repChance = 1;
        n->next = NULL;

        if (tab->head == NULL)
            tab->head = n;
        else
        {
            tmp = tab->head;

            while (tmp->next != NULL)
                tmp = tmp->next;
            tmp->next = n;
        }
    }
    else
        printf("Error pushing with Clock struct fifo\n");
}

int main(int argc, char** argv){

  if(argc != 6){
    printf("\nInput: [VMsimulator] [plist] [ptrace] [int(size of pages)] [replacementAlgorithm] [+/-]");
    return -1;
  }

  char* plist = argv[1];  //set plist path
  char* ptrace = argv[2]; //set ptrace path
  int pageSize = atoi(argv[3]);
  //check for correct pageSize: lessthan 0, power of 2, max page.
  //***
  char* replacementAlgorithm = argv[4]; //replacement Algorithm Method
  if (strcmp(replacementAlgorithm, "FIFO") && strcmp(replacementAlgorithm, "LRU") && strcmp(replacementAlgorithm, "Clock")){
    printf("\nInput: incorrect replacementAlgorithm: (options) FIFO, LRU, or Clock");
    return -1;
  }
  int prePaging;  //prePaging initialize
  if(!strcmp(argv[5], "+")){
    prePaging = 1;
  }else if(!strcmp(argv[5], "-")){
    prePaging = 0;
  }else{
    printf("\nInput: incorrect prePaging: (options) + or -");
    return -1;
  }

  int pSwapCounter = 0;

  //read files
  FILE* plistFile = fopen("plist.txt", "r");
  FILE* ptraceFile = fopen("ptrace.txt", "r");

  if(plistFile == NULL || ptraceFile == NULL){
    printf("\nError: file location");
    return -1;
  }

  //count the number of processes in plistFile
  int hol = 0;
  int k = 0;
  char c;
  while(!feof(plistFile)){
    c = fgetc(plistFile);
    k++;
    if(c == '\n'){
      if(k > 1){
        k = 0;
        hol++;
      }
    }
  }
  int processCounter = hol;

  struct pageTable** pageTables;

  int i = 0;
  int j = 0;
  char* line = NULL;
  size_t len = 0;
  ssize_t motal;
  struct pageTable* temp_table;

  if(processCounter > 0){
    pageTables = (struct pageTable**)malloc(sizeof(struct pageTable*)*processCounter);
    if(pageTables == NULL){
      printf("\nError: memory allocation");
      return -1;
    }
    //rewind
    rewind(plistFile);

    //set up table with rewinded plistFile
    while((motal = getline(&line, &len, plistFile)) != -1){
      //ignore empty lines
      if(!strcmp(line, "\n")){
        continue;
      }

      strtok(line, " ");
      pageTables[i] = createPageTable(pageSize, atoi(strtok(NULL, " ")));
      if(pageTables[i] == NULL){
        printf("\nError: memory allocation");
        return -1;
      }
      i++;
    }

    int initMem = (int)floor((double)512/(processCounter * pageSize));

    for(i = 0; i < processCounter; i++){
      for(j = 0; j < initMem && j < pageTables[i]->numPages; j++){
        pageTables[i]->pages[j]->validBit = 1;
        pageTables[i]->numLoaded++;

        struct fifo* temp;
        struct fifo* n;
        //if algorithm: FIFO
        if(!strcmp(replacementAlgorithm, "FIFO")){
          n = (struct fifo*)malloc(sizeof(struct fifo));
          if(n != NULL){
              n->pageNumber = j;
              n->next = NULL;
              if(pageTables[i]->head == NULL){
                pageTables[i]->head = n;
              } else{
                temp = pageTables[i]->head;
                while(temp->next != NULL){
                  temp = temp->next;
                }
                temp->next = n;
              }
          } else{
            printf("Error: memory allocation with FIFO");
          }
        }
        //if algorithm: Clock
        if(!strcmp(replacementAlgorithm, "Clock")){
          n = (struct fifo*)malloc(sizeof(struct fifo));
          if( n != NULL){
            n->pageNumber = j;
            n->repChance = 1;
            n->next = NULL;

            if(pageTables[i]->head == NULL){
              pageTables[i]->head = n;
            } else{
              temp = pageTables[i]->head;
              while(temp->next != NULL){
                temp = temp->next;
              }
              temp->next = n;
            }
          }
        }
      }
    }

    //ptraceFile
    line = NULL;
    len = 0;
    int cycle = 1;
    while((motal = getline(&line, &len, ptraceFile)) != -1){
      if(!strcmp(line, "\n")){
        continue;
      }
      // i = process number, j = access number
      i = atoi(strtok(line, " "));
      temp_table = pageTables[i];
      j = atoi(strtok(NULL, " "));
      j = (int)ceil((double)j/pageSize);
      j--;

      //valid pages
      if(temp_table->pages[j]->validBit){
        //if algorithm: LRU
        if(!strcmp(replacementAlgorithm, "LRU")){
          temp_table->pages[j]->accessed = cycle;
        }
        //if algorithm: Clock
        if(!strcmp(replacementAlgorithm, "Clock")){
          struct fifo* temp;
          if(temp_table->head != NULL){
            temp = temp_table->head;
            while(temp->pageNumber != j && temp->next != NULL){
              temp = temp->next;
            }
            if(temp->pageNumber == j){
              temp->repChance = 1;
            }
          }
        }
      //pages not valid
      } else{
        pageSwapCount++;

        if(prePaging){
          //if algorithm: FIFO
          if(!strcmp(replacementAlgorithm, "FIFO")){
            i = popFifo(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }
            i = popFifo(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }
            if(temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              pushFifo(temp_table, j);
            }
            //next page
            j++;
            if(j == temp_table->numPages){
              j = 0;
            }

            k = j;
            while(temp_table->pages[j]->validBit){
              j = (j+1) % temp_table->numPages;
              if(j == k){
                j = -1;
                break;
              }
            }

            if(j != -1 && temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              pushFifo(temp_table, j);
            }
          }

          //if algorithm: LRU
          if(!strcmp(replacementAlgorithm, "LRU")){
            i = indexOfLRUValidPage(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }
            i = indexOfLRUValidPage(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }

            if(temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              temp_table->pages[j]->accessed = cycle;
            }
            j++;
            if(j == temp_table->numPages){
              j = 0;
            }

            k = j;
            while(temp_table->pages[j]->validBit){
              j = (j+1) % temp_table->numPages;
              if(j == k){
                j = -1;
                break;
              }
            }

            if(j != -1 && temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              temp_table->pages[j]->accessed = cycle;
            }
          }

          //if algorithm: Clock
          if(!strcmp(replacementAlgorithm, "Clock")){
            i = popClock(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }
            i = popClock(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }

            if(temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              pushClock(temp_table, j);
            }
            j++;
            if(j == temp_table->numPages){
              j = 0;
            }

            k = j;
            while(temp_table->pages[j]->validBit){
              j = (j+1) % temp_table->numPages;
              if(j == k){
                j = -1;
                break;
              }
            }

            if(j != -1 && temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              pushClock(temp_table, j);
            }
          }
        } else{

          if(!strcmp(replacementAlgorithm, "FIFO")){
            i = popFifo(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }
            if(temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              pushFifo(temp_table, j);
            }
          }

          if(!strcmp(replacementAlgorithm, "LRU")){
            i = indexOfLRUValidPage(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }
            if(temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              temp_table->pages[j]->accessed = cycle;
            }
          }

          if(!strcmp(replacementAlgorithm, "Clock")){
            i = popClock(temp_table);
            if(i > -1){
              temp_table->pages[i]->validBit = 0;
              temp_table->numLoaded--;
            }
            if(temp_table->numLoaded < initMem){
              temp_table->pages[j]->validBit = 1;
              temp_table->numLoaded++;
              pushClock(temp_table, j);
            }
          }
        }
      }

      if(temp_table->numLoaded > initMem){
        printf("\nError: Not enough memory\n");
        return -1;
      }

      cycle++;
    }
  }

  fclose(plistFile);
  fclose(ptraceFile);

  printf("\nTotal Page Swaps: %lu\n", pageSwapCount);

  return 0;
}
