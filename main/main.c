
#include <mkrtos/task.h>
#define __LIBRARY__
//#include <unistd.h>
#include <stdio.h>

//static inline _syscall0(int,setup);
int main(void)
{
    setup();
//    fopen("/dev","r");
//    printf("test");
	while(1);
}
//MODE COM2 115200,0,8,1
//ASSIGN COM2 <S1IN>S1OUT


