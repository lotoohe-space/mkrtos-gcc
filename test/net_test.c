//
// Created by Administrator on 2022/4/7.
//

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
    close(fd);
    printf("exit.\n");
}

void net_test(void)
{
    int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
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
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
        if(connfd<0){
            printf("accept errno %d.\n",connfd);
            exit(0);
        }
        int ret_pid=fork();
        if(ret_pid<0){
            printf("fork error.\n");
        }else if(ret_pid==0){
            printf("fd is %d.\n",connfd);
            sock_sock(connfd);
        }else if(ret_pid>0){
            close(connfd);
            waitpid(ret_pid,0,0);
        }
    }
    printf("close socket.\n");
}