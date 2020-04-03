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
#include <ftw.h>
#define MAXFD 20
#define ERR(source) (perror(source),\
		     fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
		     exit(EXIT_FAILURE))


void usage(char* pname){
	fprintf(stderr,"USAGE:%s -n Name -p OCTAL -s SIZE\n",pname);
	exit(EXIT_FAILURE);
}

int usun(const char *name, const struct stat *s, int type, struct FTW *f)
{
	        switch (type){
                case FTW_DNR:
                case FTW_D: rmdir(name); break;
                case FTW_F: remove(name); break;
                case FTW_SL: unlink(name); break;
                default : ERR("wtf");
        }

	return 0;
}


int main(int argc, char** argv)
{
    FILE* s1;
    char* dirname = "Nowa sciezka";
    DIR* dir = opendir(dirname);
    if(dir)
        {
            printf("directory already exist - removing\n");
            if(nftw(dirname,usun,MAXFD,FTW_PHYS)==0)
            if(rmdir(dirname)) ERR("rmdir");
        } 
        printf("make directory\n");
        if(mkdir(dirname, 0777)) ERR("mkdir");
        
        int MAX_PATH = 1000;
        char buf[100];
        char path[MAX_PATH];
	    if(getcwd(path, MAX_PATH)==NULL) ERR("getcwd");
        if(chdir(dirname)) ERR("chdir");
        char *result = malloc(strlen(path) + strlen(dirname) + 1);
        for(int i = 0; i < 10; i++)
        {
            if(i%2==0)
            {
            if(sprintf(buf, "%i", i)<0) ERR("itoa");
            if((s1=fopen(buf,"w+"))==NULL) ERR("fopen");
            if(fclose(s1)) ERR("fclose");
            }else
            {
                strcpy(result, path);
                strcat(result,"/");
                strcat(result, dirname);
   
                if(sprintf(buf, "%i", i)<0) ERR("sprintf");
                if(symlink(result,buf)) ERR("symlink");

            }
            
           
        }
        if(chdir(path)) ERR("chdir");


    return EXIT_SUCCESS;
} 
