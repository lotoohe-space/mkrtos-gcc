//
// Created by Administrator on 2022/3/9.
//
#include <termios.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
struct termios oldtermios;
static void write1(char* w){
    write(1,w,strlen(w));
}
void die(const char *message) {
    write1(message);
    write1("\n");
    exit(1);
}
static void echo_off() {
    struct termios foo;
    if (tcgetattr(0,&oldtermios))
        die("tcgetattr failed");
    foo=oldtermios;
    foo.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
    tcsetattr(0, TCSANOW, &foo);
}

static void echo_on() {
    oldtermios.c_lflag |= ECHO | ECHOE | ECHOK;
    tcsetattr(0, TCSANOW, &oldtermios);
}

static char username[32]={0};
static char pwd[32]={0};
int login(void){
    int r_len;
    again:
    write1("login:");
    r_len=read(0,username,sizeof(username)-1);
    if (r_len<0) die("read failed");
    username[r_len-1]=0;
    write1("please input pwd:");
    echo_off();
    r_len=read(0,pwd,sizeof(username)-1);
    if (r_len<0) die("read failed");
    pwd[r_len-1]=0;
    echo_on();
    write1("\n");
    if(strcmp(username,"root")==0
        &&strcmp(pwd,"root")==0
    ) {

        return 0;
    }
    write1("login error.\n");
    write1("\n");
    goto again;
}