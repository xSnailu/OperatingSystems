#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <aio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
int BLOCKS;
int AIOCB_NUMBER;
#define SHIFT2(counter, x) ((counter + x) % 2)
#define SHIFT3(counter, x) ((counter + x) % 3)
void error(char *);
void usage(char *);
void siginthandler(int);
void sethandler(void (*)(int), int);
off_t getfilelength(int);
void fillaiostruct(struct aiocb *, char *, int, int);
void suspend(struct aiocb *);
void readdata(struct aiocb *, off_t);                
void writedata(struct aiocb *, off_t);
void syncdata(struct aiocb *);
void cleanup(char **, int);
void reversebuffer(char *, int);
void processblocks(struct aiocb *, char **, int, int, int);
volatile sig_atomic_t work;
void error(char *msg){
	perror(msg);
	exit(EXIT_FAILURE);
}
void usage(char *progname){
	fprintf(stderr, "%s bad arguments\n", progname);
	fprintf(stderr, "file1 - input file name\n");
	fprintf(stderr, "file2 - output file name\n");
	fprintf(stderr, "n - line number\n");
	exit(EXIT_FAILURE);
}

void siginthandler(int sig){
	
}
void sethandler(void (*f)(int), int sig){
	struct sigaction sa;
	memset(&sa, 0x00, sizeof(struct sigaction));
	sa.sa_handler = f;
	if (sigaction(sig, &sa, NULL) == -1)
		error("Error setting signal handler");
}
off_t getfilelength(int fd){
	struct stat buf;
	if (fstat(fd, &buf) == -1)
		error("Cannot fstat file");
	return buf.st_size;
}
void suspend(struct aiocb *aiocbs){
	struct aiocb *aiolist[1];
	aiolist[0] = aiocbs;
	
	while (aio_suspend((const struct aiocb *const *) aiolist, 1, NULL) == -1){
		
		if (errno == EINTR) continue;
		error("Suspend error");
	}
	if (aio_error(aiocbs) != 0)
		error("Suspend error");
	if (aio_return(aiocbs) == -1)
		error("Return error");
}
void fillaiostruct(struct aiocb *aiocbs, char *buffer, int fd, int blocksize)
{
		memset(aiocbs, 0, sizeof(struct aiocb));
		aiocbs->aio_fildes = fd;		
		aiocbs->aio_offset = 0;
		aiocbs->aio_nbytes = blocksize;
		aiocbs->aio_buf = (void *) buffer;
		aiocbs->aio_sigevent.sigev_notify = SIGEV_NONE;
}
void readdata(struct aiocb *aiocbs, off_t offset){
	
	aiocbs->aio_offset = offset;
	if (aio_read(aiocbs) == -1)
		error("Cannot read");
}
void writedata(struct aiocb *aiocbs, off_t offset){
	
	aiocbs->aio_offset = offset;
	if (aio_write(aiocbs) == -1)
		error("Cannot write");
}
void syncdata(struct aiocb *aiocbs){
	
	suspend(aiocbs);
	if (aio_fsync(O_SYNC, aiocbs) == -1)
		error("Cannot sync\n");
	suspend(aiocbs);
}

void cleanup(char **buffers, int fd){
	int i;
	if (aio_cancel(fd, NULL) == -1)
			error("Cannot cancel async. I/O operations"); 
	for (i = 0; i<BLOCKS; i++)
		free(buffers[i]);
	if (TEMP_FAILURE_RETRY(fsync(fd)) == -1)
		error("Error running fsync");  
}
void reversebuffer(char *buffer, int blocksize){
	int k;
    char tmp;
    for (k = 0; k < blocksize / 2; k++)
	{
                tmp = buffer[k];
                buffer[k] = buffer[blocksize - k - 1];
                buffer[blocksize - k - 1] = tmp;
    }
}
void processblocks(struct aiocb *aiocbs, char **buffer, int bcount, int bsize, int iterations){
}



int main(int argc, char *argv[])
{

	BLOCKS = 3;
	AIOCB_NUMBER = 3;
    char* inputfilename;
	char* outputfilename;
	int byte_number;
	if (argc != 4)
		usage(argv[0]);

	//pobieranie argumentow
	inputfilename = argv[1];
	outputfilename = argv[2];
	byte_number = atoi(argv[3]);

	

	//wyswietlanie argumentow
	fprintf(stderr, "arguments aquired\n");
	fprintf(stderr, "%s - input file name\n", inputfilename);
	fprintf(stderr, "%s - output file name\n", outputfilename);
	fprintf(stderr, "%i - line number\n", byte_number);
	
	
	
	int inputfiledescriptor;
	int outputfiledescriptor;
	
	if ((inputfiledescriptor = TEMP_FAILURE_RETRY(open(inputfilename, O_RDWR))) == -1)
		error("Cannot open input file");
	if ((outputfiledescriptor = TEMP_FAILURE_RETRY(open(outputfilename, O_RDWR))) == -1)
		error("Cannot open output file");

	int inputfilelength = getfilelength(inputfiledescriptor);
	int outputfilelength = getfilelength(outputfiledescriptor);
	
	fprintf(stderr, "%i - inputfile length\n", inputfilelength);
	fprintf(stderr, "%i - outputfile length\n", outputfilelength);

	int blocksize = 10000;
	char* buffer[3];
	if (blocksize > 0)
	{
		for (int i = 0; i<BLOCKS; i++)
			if ((buffer[i] = (char *) calloc (blocksize, sizeof(char))) == NULL)
				error("Cannot allocate memory");
		
	}

	struct aiocb aiocbs[3];
	

	fillaiostruct(&aiocbs[0],buffer[0],inputfiledescriptor,blocksize);
	fillaiostruct(&aiocbs[1],buffer[0],outputfiledescriptor,blocksize);
	fillaiostruct(&aiocbs[2],buffer[2],outputfiledescriptor,blocksize);

	//readdata(&aiocbs[0], 0);
	//suspend(&aiocbs[0]);

	readdata(&aiocbs[2], 0);
	suspend(&aiocbs[2]);

	//printf("input1: %s\n",buffer[0]);
	//reversebuffer(buffer[0],2);
	//printf("revinput1: %s\n",buffer[0]);
	printf("output: %s\n",buffer[2]);

	//writedata(&aiocbs[1],0);
	//suspend(&aiocbs[1]);
	fillaiostruct(&aiocbs[2],buffer[2],outputfiledescriptor,3);
	writedata(&aiocbs[2],0);
	suspend(&aiocbs[2]);
	
	if (TEMP_FAILURE_RETRY(close(inputfiledescriptor)) == -1)
		error("Cannot close file");
	if (TEMP_FAILURE_RETRY(close(outputfiledescriptor)) == -1)
		error("Cannot close file");



	
	return EXIT_SUCCESS;
}
