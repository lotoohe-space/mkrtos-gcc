#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include "sys/wait.h"

static char sendBuff[512];

static void sock_sock(int fd){
    printf("to write data.\n");
   while(1) {
        if(fd==128){
            printf("fd is error.\n");
        }
        int ret = write(fd, sendBuff, sizeof(sendBuff));
        if (ret < 0) {
            printf("write errno %d.\n",ret);
            if (errno != EINTR) {
                break;
            }
        }
    }
//   close(fd);
//    int ret = write(fd, sendBuff, sizeof(sendBuff));
    close(fd);
    printf("exit.\n");
 //   exit(0);
}

int net_main(void)
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;


    time_t ticks;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8777);

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);

    while(1)
    {
       // printf("accept..\n");
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if(connfd<0){
            printf("accept errno %d.\n",connfd);
            exit(0);
//         continue;
        }
//        int val;
//        //setsockopt(connfd,SOL_SOCKET,SO_KEEPALIVE,&val,sizeof(val));
//       // printf("%d linked.\n",connfd);
        int ret_pid=fork();
        if(ret_pid<0){
            printf("fork error.\n");
        }else if(ret_pid==0){
          //  printf("child pid is %d.\n",getpid());
          printf("fd is %d.\n",connfd);
            sock_sock(connfd);
        }else if(ret_pid>0){
            close(connfd);
            //printf("father pid is %d.\n",getpid());
            waitpid(ret_pid,0,0);
          //  printf("to accept\n");
        }
       //ticks = time(NULL);
        //snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));

//        struct timespec times;
//        times.tv_sec=0;
//        times.tv_nsec=10*1000*1000;
//        nanosleep(&times,NULL);
        //sleep(1);
    }
    printf("close socket.\n");
}