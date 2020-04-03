#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     exit(EXIT_FAILURE))

//MAX_BUFF must be in one byte range
#define MAX_BUFF 200
//NUMBER_OF_PIPES is number of all processes (with parent process)
//each 2 procesess need two pipes, so if there are 3 processes pipes look like this: 1-2 2-3 3-1 
#define NUMBER_OF_PIPES 4

volatile sig_atomic_t last_signal = 0;


int sethandler( void (*f)(int), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1==sigaction(sigNo, &act, NULL))
		return -1;
	return 0;
}

//unnecessary handlers 
/*
void sig_handler(int sig) {
	last_signal = sig;
}

void sig_killme(int sig) {
	if(rand()%5==0) 
		exit(EXIT_SUCCESS);
}


void sigchld_handler(int sig) {
	pid_t pid;
	for(;;){
		pid=waitpid(0, NULL, WNOHANG);
		if(0==pid) return;
		if(0>=pid) {
			if(ECHILD==errno) return;
			ERR("waitpid:");
		}
	}
}
*/

//close all descriptors in fds in range from 0 to fds_size( typically NUMBER_OF_PIPES*2) except descriptors assigned to iterators read_i, write_i 
void close_pipes_except(int read_i, int write_i, int* fds, int fds_size){
	for(int i=0; i<fds_size; i++){
		if(i!=read_i&&i!=write_i){
			printf("process pid: %i ---- descriptor number %i closed. \n",getpid(),fds[i]);
			if(TEMP_FAILURE_RETRY(close(fds[i]))) ERR("close");
		}
	}
	printf("\n-----------------------------\n");
}

void child_work(int read_R, int write_R) {
	int status;

	char buffer[MAX_BUFF];
	char *buf;
	int received_number;
	int new_number;
	srand(getpid());	

	for(;;){
		
		sleep(1);
		status=read(read_R,buffer,MAX_BUFF);
		if(status<0&&errno==EINTR) continue;
		if(status<0) ERR("read header from R");
		if(0==status) break;
		received_number = *((int*)buffer);
		printf("my pid is: %i ---- i received number: %i!\n",getpid(),received_number);

		//STOP
		if(received_number>999){
			printf("#########################\nOh shit received_number is bigger than 999! Ight Imma Head Out!\n#########################\n");
			break;
		}

		new_number = rand()%25;
		new_number = new_number - 10;
		*((int *)buffer)=received_number + new_number;
		buf=buffer+sizeof(int);
		memset(buf,0,MAX_BUFF - sizeof(int));
		if(TEMP_FAILURE_RETRY(write(write_R,buffer,MAX_BUFF)) <0) ERR("write to R");
	}
}

void parent_work(int read_R, int write_R) {
	child_work(read_R, write_R);
}

void parent_first_work(int read_R, int write_R) {
	char c1 = 'b';
	char c2;
	int status;

	
	int start_number = 3;

	printf("i am parent and i will start with writing 1\n");

	char buffer[MAX_BUFF];
	char *buf;
	*((int *)buffer)=start_number;
	buf=buffer+sizeof(int);
	memset(buf,0,MAX_BUFF - sizeof(int));
	if(TEMP_FAILURE_RETRY(write(write_R,buffer,MAX_BUFF)) <0) ERR("write to R");
}

pid_t create_new_process(int *fds, int read_i, int write_i){

	pid_t pid_child = fork();

	switch (pid_child) {
			case 0:
			close_pipes_except(read_i,write_i,fds,NUMBER_OF_PIPES*2);
			child_work(fds[read_i], fds[write_i]);

			printf("my pid is: %i ---- descriptor: %i closed\n",getpid(),fds[read_i]);
				if(TEMP_FAILURE_RETRY(close(fds[read_i]))) ERR("close");
			printf("my pid is: %i ---- descriptor: %i closed\n",getpid(),fds[write_i]);
				if(TEMP_FAILURE_RETRY(close(fds[write_i]))) ERR("close");
			
			printf("my pid is: %i ---- my job is done.\n",getpid());
			exit(EXIT_SUCCESS);

			case -1: ERR("Fork:");
		}
	return pid_child;
}





void usage(char * name){
	fprintf(stderr,"USAGE: %s n\n",name);
	fprintf(stderr,"null args!");
	exit(EXIT_FAILURE);
}





int main(int argc, char** argv) {
	pid_t pid_table[NUMBER_OF_PIPES-1];
	if(1!=argc) usage(argv[0]);

	int *fds;
	if(NULL==(fds=(int*)malloc(sizeof(int)*NUMBER_OF_PIPES*2))) ERR("malloc");
	
	for(int i=0; i<NUMBER_OF_PIPES; i++){
		if(pipe(fds + i*2)) ERR("pipe");
	}

	for(int i=0; i<NUMBER_OF_PIPES*2; i++){
		printf("my pid is: %i ---- descriptor: %i CREATED\n",getpid(),fds[i]);
	}


	printf("\n-----------------------------\n");
	
	
	//if(sethandler(SIG_IGN,SIGINT)) ERR("Setting SIGINT handler");
	if(sethandler(SIG_IGN,SIGPIPE)) ERR("Setting SIGINT handler");
	//if(sethandler(sigchld_handler,SIGCHLD)) ERR("Setting parent SIGCHLD:");
	
	for(int i=0; i<NUMBER_OF_PIPES-1;i++){
		pid_table[i] = create_new_process(fds, i*2, (i*2 + 3)%(NUMBER_OF_PIPES*2) );
		sleep(1);
	}

	close_pipes_except(NUMBER_OF_PIPES*2 -2, (NUMBER_OF_PIPES*2 + 1)%(NUMBER_OF_PIPES*2) ,fds,NUMBER_OF_PIPES*2);
	

	parent_first_work(fds[NUMBER_OF_PIPES*2 - 2], fds[(NUMBER_OF_PIPES*2 + 1)%(NUMBER_OF_PIPES*2) ]);
	parent_work(fds[NUMBER_OF_PIPES*2 - 2], fds[(NUMBER_OF_PIPES*2 + 1)%(NUMBER_OF_PIPES*2) ]);

	printf("my pid is: %i ---- descriptor: %i closed\n",getpid(),fds[NUMBER_OF_PIPES*2 - 2]);
		if(TEMP_FAILURE_RETRY(close(fds[NUMBER_OF_PIPES*2 - 2]))) ERR("close");
	printf("my pid is: %i ---- descriptor: %i closed\n",getpid(),fds[(NUMBER_OF_PIPES*2 + 1)%(NUMBER_OF_PIPES*2)]);
		if(TEMP_FAILURE_RETRY(close(fds[(NUMBER_OF_PIPES*2 + 1)%(NUMBER_OF_PIPES*2)]))) ERR("close");
20
		printf("pid to catch: %i \n", pid_table[i]);

	for(int i=0; i<NUMBER_OF_PIPES-1; i++){
		sleep(1);
		pid_t pid;
		pid=waitpid(pid_table[i], NULL, WNOHANG);

		if(0>=pid) {
				if(ECHILD==errno) break;
				ERR("waitpid:");
			}
		printf("process %i catched \n", pid_table[i]);
	}

	


	printf("parent's job is done!\n");
	return EXIT_SUCCESS;
}
