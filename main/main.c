
#include <mkrtos/task.h>
#define __LIBRARY__
#include <unistd.h>

static inline _syscall0(int,setup);
int main(void)
{

    setup();
	while(1);
}
//MODE COM2 115200,0,8,1
//ASSIGN COM2 <S1IN>S1OUT


