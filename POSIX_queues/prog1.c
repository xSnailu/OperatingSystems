#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>

#define LIFE_SPAN 10
#define MAX_NUM 10

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
                     perror(source),kill(0,SIGKILL),\
		     		     exit(EXIT_FAILURE))

volatile sig_atomic_t children_left = 0;

void sethandler( void (*f)(int, siginfo_t*, void*), int sigNo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_sigaction = f;
	act.sa_flags=SA_SIGINFO;
	if (-1==sigaction(sigNo, &act, NULL)) ERR("sigaction");
}

void mq_handler(int sig, siginfo_t *info, void *p) {
	printf("cawabunga\n");
	mqd_t *pin;
    uint8_t ni;
    unsigned msg_prio;

    pin = (mqd_t *)info->si_value.sival_ptr;

    static struct sigevent not;
    not.sigev_notify=SIGEV_SIGNAL;
    not.sigev_signo=SIGRTMIN;
    not.sigev_value.sival_ptr=pin;
    if(mq_notify(*pin, &not)<0)ERR("mq_notify");

	char buf[256];
    for(;;){
            if(mq_receive(*pin,buf,256,&msg_prio)<1) {
                       if(errno==EAGAIN) break;
                       else ERR("mq_receive");
            }
            printf("Recieved message: %s", buf );
        }
}

void sigchld_handler(int sig, siginfo_t *s, void *p) {
	pid_t pid;
	for(;;){
		pid=waitpid(0, NULL, WNOHANG);
		if(pid==0) return;
		if(pid<=0) {
			if(errno==ECHILD) return;
			ERR("waitpid");
		}
		children_left--;
	}
}

void child_work(int n,mqd_t pin, mqd_t pout) {
	int life;
	uint8_t ni;
	uint8_t my_bingo;
	srand(getpid());
	life = rand()%LIFE_SPAN+1; 
	my_bingo=(uint8_t)(rand()%MAX_NUM);
	while(life--){
		if(TEMP_FAILURE_RETRY(mq_receive(pout,(char*)&ni,1,NULL))<1)ERR("mq_receive");
		printf("[%d] Received %d\n",getpid(),ni);
		if(my_bingo==ni) {
			if(TEMP_FAILURE_RETRY(mq_send(pin,(const char*)&my_bingo,1,1)))ERR("mq_send");
			return;
		}
	}
	if(TEMP_FAILURE_RETRY(mq_send(pin,(const char*)&n,1,0)))ERR("mq_send");
}


void parent_work(mqd_t pout) {
	srand(getpid());
	uint8_t ni;
	while(children_left){
		ni=(uint8_t)(rand()%MAX_NUM);
		if(TEMP_FAILURE_RETRY(mq_send(pout,(const char*)&ni,1,0)))ERR("mq_send");
		sleep(1);
	}
	printf("[PARENT] Terminates \n");
}

void create_children(int n, mqd_t pin, mqd_t pout) {
	while (n-->0) {
		switch (fork()) {
			case 0: child_work(n,pin,pout);
				exit(EXIT_SUCCESS);
			case -1:perror("Fork:");
				exit(EXIT_FAILURE);
		}
		children_left++;
	}
}

void usage(void){
	fprintf(stderr,"c PID - nadpisanie PIDow kolejek do ktorych program wysyla wiadomosci. \n");
	fprintf(stderr,"s TEXT - wysylanie widomosci do zarejestrowanych odbiorcow \n");
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	
	if(argc!=2) usage();
	char *mq_name;
	mq_name = argv[1];
	
	mqd_t pid_send_list[2];
	pid_send_list[0] = 0;
	pid_send_list[1] = 0;
	

	mqd_t pid_in;
	//atrybuty
	struct mq_attr attr;
	attr.mq_maxmsg=4;
	attr.mq_msgsize=256;
	//otwarcie kolejki do czytania
	mq_unlink(mq_name);
	if((pid_in=TEMP_FAILURE_RETRY(mq_open(mq_name, O_RDWR | O_CREAT | O_NONBLOCK, 0600, &attr)))==(mqd_t)-1) ERR("mq open out");
	printf("Stworzylem kolejke PID_in. Moj PID to: %d. Nazwa kolejki: %s.\n",getpid(), mq_name);
	
	//ustawianie handlera dla sygnału który będzie szedł razem z mq_notify
	sethandler(mq_handler,SIGRTMIN);

	//konfiguracja eventu dla mq_notify
	static struct sigevent not;
	not.sigev_notify=SIGEV_SIGNAL;
	not.sigev_signo=SIGRTMIN;
	not.sigev_value.sival_ptr=&pid_in;
	if(mq_notify(pid_in, &not)<0)ERR("mq_notify");


	
    
	int pid_send_list_position = 0;
    char buffer[256];

    while (1)
    {
		
		TEMP_FAILURE_RETRY( fgets ( buffer, 256, stdin ) != NULL );
		
		if (*buffer=='\0')
		{
			continue;
		}
		
		printf("%s\n",buffer);
		
		if(buffer[0] != 'c' || buffer[0] != 's')
			if(buffer[1] != ' ')
				usage();
   		
		char switch_char = buffer[0];
		switch (switch_char)
		{
		case 'c':
		fputs ( "Otrzymalem polecenie dodania PIDu.\n", stdout ); 
		mq_name = buffer + 2;
		*(mq_name + strlen(mq_name)-1) = '\0';
		printf  ( "Dodaje PID o numerze: %s do listy.\n", mq_name ); 
		if((pid_send_list[pid_send_list_position]=TEMP_FAILURE_RETRY(mq_open(mq_name, O_RDWR | O_CREAT, 0600, &attr)))==(mqd_t)-1) ERR("mq open out");
		pid_send_list_position++;
		pid_send_list_position = pid_send_list_position % 2;
		printf("pid_send_list[0]= %i\n", pid_send_list[0]);
		printf("pid_send_list[1]= %i\n", pid_send_list[1]);
			break;
		case 's':
		// send message from stdin to recievers
		if(pid_send_list[0]!=0)
			if(TEMP_FAILURE_RETRY(mq_send(pid_send_list[0],buffer + 2,strlen(buffer + 2),1)))ERR("mq_send");
		if(pid_send_list[1]!=0)
			if(TEMP_FAILURE_RETRY(mq_send(pid_send_list[1],buffer + 2,strlen(buffer + 2),1)))ERR("mq_send");
		//if(mq_notify(pid_in, &not)<0)ERR("mq_notify");
			break;
		
		default:
			usage();
			break;
		}
		*buffer='\0';
    } 
	


	

	//parent_work(pout);

	mq_close(pid_in);
	if(mq_unlink("/PID_in"))ERR("mq unlink");
	
	return EXIT_SUCCESS;
}
