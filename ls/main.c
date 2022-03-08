#include <stdio.h>
#include "arm/syscalls.h"
#include "dirent.h"
#include "dietdirent.h"
#include "fcntl.h"
int errno=0;

int _start(int argc,char *argv[],char *env[]) {
    int i;
//    printf("\n=========I am a process image!============\n");
//    printf("\nMy pid = %d,  parentpid = %d\n", getpid(), getppid());
//    for(i=0; i< argc; i++)
//        printf("\nargv[%d]:  %s\n",i ,argv[i]);
//    printf("\n============= process =================\n\n");

    DIR				*dp;
    struct dirent	*dirp;
    if(argc<=1){
        return -1;
    }
//    for(int i=0;i<1000000;i++){
//        argc=argc;
//    }
    if ((dp = opendir(argv[1])) == NULL) {
        printf("to open %s is fail.\n",argv[1]);
        return -1;
    }
//    printf("opendir d:%x fd:%x.\n" ,dp,dp->fd);
    putchar('\n');
    while ((dirp = readdir(dp)) != NULL) {
//        printf("readdir d:%x fd:%x.\n" ,dp,dp->fd);
//        printf("readdir %x.\n",dirp);
        printf("%s\t", dirp->d_name);
    }
//    printf("putchar\n");
    putchar('\n');
//    printf("closedir d:%x fd:%x.\n" ,dp,dp->fd);
    closedir(dp);
//    printf("exit\n");
    return 0;
}
