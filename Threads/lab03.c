#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))

void handler(int signo)
{
  return;
}

typedef unsigned int UINT;
typedef struct argsEstimation
 {
        pthread_t tid;
        UINT seed;
        double x1;
        double x2;
        int k;
        sigset_t *pMask;
 }argsEstimation_t;

 typedef struct rhandler
 {
        pthread_t tid;
        argsEstimation_t * tids_table;
        int* tids_table_size;
        sigset_t *pMask;
 }rhandler_t;


void ReadArguments(int argc, char **argv, int *threadCount, int *pointsCount, double *x1, double *x2);
void* thread_funct(void *args);
double f(double arg);
void* r_handler_f(void *args);


void sig_handler(int sig) {
	printf("1000ms do konca\n");
}

int main(int argc, char** argv)
{
    
    int threadCount, pointsCount;
    double x1, x2;

    ReadArguments(argc, argv, &threadCount, &pointsCount, &x1, &x2);
    argsEstimation_t* estimations = (argsEstimation_t*) malloc(sizeof(argsEstimation_t) * threadCount);
    if (estimations == NULL) ERR("Malloc error for estimation arguments!");

    srand(time(NULL));


    sigset_t oldMask, newMask;
	sigemptyset(&newMask);
	sigaddset(&newMask, SIGINT);
    if (pthread_sigmask(SIG_BLOCK, &newMask, &oldMask)) ERR("SIG_BLOCK error");

    for (int i = 0; i < threadCount; i++) {
                    estimations[i].pMask =  &newMask;
                    estimations[i].seed = rand();
                    estimations[i].x1 = x1;
                    estimations[i].x2 = x2;
                    estimations[i].k = pointsCount;
            }      

    
    rhandler_t* rhandler = (rhandler_t*)malloc(sizeof(rhandler_t));
    rhandler->pMask = &newMask;
    rhandler->tids_table = estimations;
    rhandler->tids_table_size = (int*)malloc(sizeof(int));
    *rhandler->tids_table_size = threadCount;

    
    printf("thread rhandler created\n");
    int err = pthread_create(&rhandler->tid, NULL, r_handler_f, rhandler);
    if (err != 0) ERR("Couldn't create thread");



    for (int i = 0; i < threadCount; i++) {
            printf("thread created\n");
            int err = pthread_create(&(estimations[i].tid), NULL, thread_funct, &estimations[i]);
            if (err != 0) ERR("Couldn't create thread");
        }

    double *subresult;
    double cumulativeResult = 0.0;

    
    
    

    for (int i = 0; i < threadCount; i++)
            {
            int err = pthread_join(estimations[i].tid, (void*)&subresult);
            if (err != 0) ERR("Can't join with a thread");
                if(NULL!=subresult)
                {
                    cumulativeResult += *subresult;
                    
                    free(subresult);
                }
           }
        printf("Cumulative result: %f\n", cumulativeResult);
        printf("%f / %i\n", cumulativeResult, threadCount);
        cumulativeResult = cumulativeResult / threadCount;
        printf("%f * (%f -  %f)\n", cumulativeResult, x2, x1);
        cumulativeResult = cumulativeResult * (x2 - x1);

        printf("Calka obliczona metoda Monte Carlo wynosi: %f\n", cumulativeResult);
}

void ReadArguments(int argc, char **argv, int *threadCount, int *pointsCount, double *x1, double *x2) 
{       
        if(argc!=5)
            printf("Invalid number of arguments");


	
		*threadCount = atoi(argv[1]);
		if (*threadCount <= 0)
         {
			printf("Invalid value for 'threadCount'");
			exit(EXIT_FAILURE);
         }
	
	
		*pointsCount = atoi(argv[2]);
		if (*pointsCount <= 0)
         {
			printf("Invalid value for 'samplesCount'");
			exit(EXIT_FAILURE);
		 }
    
        *x1 = atoi(argv[3]);
        *x2 = atoi(argv[4]);
}

void* thread_funct(void *voidPtr)
{
	
    struct timespec ts;
    ts.tv_sec = 100 / 1000;
    ts.tv_nsec = (100 % 1000) * 1000000;
    nanosleep(&ts, NULL);


    double* result;
	if(NULL==(result=malloc(sizeof(double)))) ERR("malloc");
    *result = 0;

    argsEstimation_t *args = voidPtr;
    
    double random_number;
    for(int i=0; i < (args->k); i++)
    {
    random_number = ((double) rand_r(&args->seed) / (double) RAND_MAX);
    double x1 = (double)(args->x1);
    double x2 = (double)(args->x2);

    random_number = (x2-x1) * random_number + x1;  
    //printf("Wylosowany punkt:%f Wartosc funkcji %f\n",random_number,f(random_number));
    nanosleep(&ts, NULL);
    *result = *result + f(random_number);
    }
    *result = *result / args->k;
    // printf("Obliczony output:%f\n",*result);
    
    return result;
}

void* r_handler_f(void *voidPtr)
{
    rhandler_t *args = voidPtr;
    char *buffer;
    size_t bufsize = 32;
    size_t characters;
    printf("%i\n",*args->tids_table_size);
    buffer = (char *)malloc(bufsize * sizeof(char));
    if( buffer == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }

    printf("Type something: ");
    while(1)
    {
        characters = getline(&buffer,&bufsize,stdin);
        printf("%zu characters were read.\n",characters);
        printf("You typed: '%s'\n",buffer);
        for(int i=0;i<bufsize;i++)
        {
            if(buffer[i]=='r')
            {
                printf("Char 'r' has been spoted!\n");
                printf("Calka jest liczona od poczatku!\n");
                for(int j=0; j<*args->tids_table_size; j++)
                    pthread_cancel(args->tids_table[i].tid);


                printf("Watki usuniete!\n");
                 for (int i = 0; i < *args->tids_table_size; i++)
                  {
                    printf("thread created\n");
                    int err = pthread_create(&(args->tids_table[i].tid), NULL, thread_funct, &args->tids_table);
                    if (err != 0) ERR("Couldn't create thread");
                  }
                
            }
        }
    }
    
    
	
    return NULL;
}

double f(double arg)
{
    return arg * arg * arg;
}
