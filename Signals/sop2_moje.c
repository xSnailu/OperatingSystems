#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

void sethandler( void (*f)(int), int sigNo) {
        struct sigaction act;
        memset(&act, 0, sizeof(struct sigaction));
        act.sa_handler = f;
        if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}
volatile sig_atomic_t sig1 = 0;
volatile sig_atomic_t sig2 = 0;

void sig1_handler(int sig) {
        printf("[%d] received signal %d\n", getpid(), sig);
        sig1++;
}

void sig2_handler(int sig) {
        printf("[%d] received signal %d\n", getpid(), sig);
        sig2++;
}




int child_work(int i) {
	for(int tt=i;tt>0;tt=sleep(tt+1));
	
	printf("pid=%i\n",getpid());
	printf("k=%i\n",i);
	srand(time(NULL)*getpid());	
	int x=1+rand()%49;
	printf("x=%i\n",x);
	
	int ile_s1 = x/10;
	int ile_s2 = x%10;
	
	for(int a=0; a<ile_s1 ; a++)
	{
		if (kill(0, SIGUSR1)<0)ERR("kill");
        struct timespec t = {0, 20*1000*1000};
        nanosleep(&t,NULL);
        
	}
	for(int b=0; b<ile_s2; b++)
	{
		if (kill(0, SIGUSR2)<0)ERR("kill");
        struct timespec t = {0, 20*1000*1000};
        nanosleep(&t,NULL);
	   
	}


	
	printf("PROCESS with pid: %d x: %i terminates\n",getpid(),x);
	return x;
	
}

void create_children(int n) {
	pid_t s;
	int i = 0;
	for (n--;n>=0;n--)
	{
		i++;
		if((s=fork())<0) ERR("Fork:");
		if(!s)
		{
			sethandler(SIG_IGN,SIGUSR1);
        	sethandler(SIG_IGN,SIGUSR2);
			exit(child_work(i));
		}
	}
}

void usage(char *name){
	fprintf(stderr,"USAGE: %s 0<n\n",name);
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	int n;
	if(argc<2)  usage(argv[0]);
	n=atoi(argv[1]);
	if(n<=0)  usage(argv[0]);
	int x;
	int tab_pid[n];
	int tab_x[n];
	sig1 = 0;
	sig2 = 0;

	sethandler(sig1_handler,SIGUSR1);
	sethandler(sig2_handler,SIGUSR2);

	create_children(n);
	int i = 0;

	while(n>0){
		pid_t pid;
		for(;;){
			pid=waitpid(0, &x, WNOHANG);
			if(pid>0)
			{
				x = WEXITSTATUS(x);
				printf("i: %i pid: %i x: %i\n",i,pid,x);
				tab_pid[i] = pid;
				tab_x[i] = x;
				printf("i: %i pid: %i x: %i sig1: %i sig2: %i\n",i,tab_pid[i],x,sig1,sig2);
				sig1 = 0;
				sig2 = 0;
				i++;
				n--;
			} 
			if(0==pid) break;
			if(0>=pid) {
				if(ECHILD==errno) break;
				ERR("waitpid:");
			}
		}
		//	printf("PARENT: %d processes remain\n",n);
	}

	n=atoi(argv[1]);
	for(int i=0; i<n; i++)
	{
		printf("pid: %i x: %i\n",tab_pid[i],tab_x[i]);
	}

	printf("sig1: %i sig2: %i\n",sig1,sig2);


	return EXIT_SUCCESS;
}