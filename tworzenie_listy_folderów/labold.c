#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ftw.h>
#define MAXFD 20
#define MAX_PATH 1000

#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))


void usage(char* pname){
	fprintf(stderr,"USAGE fail\n");
	exit(EXIT_FAILURE);
}

int usun(const char *name, const struct stat *s, int type, struct FTW *f)
{
	        switch (type){
                case FTW_DNR:
                case FTW_D: nftw(name,usun,MAXFD,FTW_PHYS); rmdir(name); break;
                case FTW_F: remove(name); break;
                case FTW_SL: unlink(name); break;
                default : ERR("wtf");
        }
        return 0;
}

int main(int argc, char** argv)
{
int c;
char rootpath[MAX_PATH];
char* rootname;
char curdir[MAX_PATH];
DIR* dir;

while ((c = getopt (argc, argv, "r:p:g::")) != -1)
		switch (c)
		{
			case 'r':
            rootname = malloc(strlen(optarg)+1);
			strcpy(rootname, optarg);
            dir = opendir(rootname);
            if(dir)
            {
            printf("\ndirectory already exist - removing\n");
            nftw(rootname,usun,MAXFD,FTW_PHYS);
            //if(rmdir(rootname)) ERR("rmdir");
            }
            mkdir(rootname, 0777);
            chdir(rootname);
           
				break;
            case 'p':
                sprintf(curdir,"%s", optarg);
                mkdir(curdir,0777);
                chdir(curdir);
                break;
                
			case '?':
			default:
				usage(argv[0]);
		}
if(argc>optind) usage(argv[0]);



printf("%s\n", rootname);
printf("%s\n", curdir);








return EXIT_SUCCESS;   
}