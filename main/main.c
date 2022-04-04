
#include <mkrtos/task.h>
#define __LIBRARY__
//#include <unistd.h>
#include <stdio.h>


extern int32_t arch_init(void);
int main(void)
{
    arch_init();

	while(1);
}
//MODE COM2 115200,0,8,1
//ASSIGN COM2 <S1IN>S1OUT


