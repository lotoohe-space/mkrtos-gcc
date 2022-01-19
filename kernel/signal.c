//
// Created by Administrator on 2022/1/9.
//
#include <arch/arch.h>
#include <type.h>
#include <mkrtos/task.h>
#include <errno.h>
void DoExit(int16_t pid,int32_t exitCode);


int32_t sys_sgetmask()
{
    return sysTasks.currentTask->signalBlocked;
}

int32_t sys_ssetmask(int32_t newmask)
{
    int32_t old=sysTasks.currentTask->signalBlocked;

    sysTasks.currentTask->signalBlocked = newmask & ~(1<<(SIGKILL-1)) & ~(1<<(SIGSTOP-1));
    return old;
}
/**
 * 这里借鉴了linux 0.11
 * @param signum
 * @param handler
 * @param restorer
 * @return
 */
int32_t sys_signal(int32_t signum, int32_t handler, int32_t restorer)
{
    struct sigaction temp;

    if (signum<1 || signum>32
            || signum==SIGKILL
            || signum==SIGSTOP
            )
        return -EINVAL;

    temp.sa_handler = (void (*)(int)) handler;
    temp.sa_mask = 0;
    temp.sa_flags = SA_ONESHOT | SA_NOMASK;
    temp.sa_restorer = (void (*)(void)) restorer;
    handler = (uint32_t) sysTasks.currentTask->signals[signum-1].sa_handler;
    sysTasks.currentTask->signals[signum-1] = temp;
    return handler;
}
// sigaction()系统调用。改变进程在收到一个信号时的操作。signum是除了SIGKILL以外的
// 任何信号。[如果新操作（action）不为空 ]则新操作被安装。如果 oldaction指针不为空，
// 则原操作被保留到oldaction。成功则返回0，否则为-EINVAL。
int sys_sigaction(int signum, const struct sigaction * action,
                  struct sigaction * oldaction)
{

    if (signum<1 || signum>32 || signum==SIGKILL || signum==SIGSTOP)
        return -EINVAL;

    sysTasks.currentTask->signals[signum-1]=*action;
    if (oldaction)
        *oldaction = sysTasks.currentTask->signals[signum-1];
    // 如果允许信号在自己的信号句柄中收到，则令屏蔽码为0，否则设置屏蔽本信号。
    if (sysTasks.currentTask->signals[signum-1].sa_flags & SA_NOMASK)
        sysTasks.currentTask->signals[signum-1].sa_mask = 0;
    else
        sysTasks.currentTask->signals[signum-1].sa_mask |= (1<<(signum-1));
    return 0;
}
/**
 * 这里处理信号
 * @param psp
 * @param signr
 * @return
 */
int32_t do_signal(void* psp,uint32_t signr){
    struct sigaction  *sig=&(sysTasks.currentTask->signals[signr - 1]);
    //忽略信号
    if(sig->sa_handler==SIG_IGN){
        return -1;
    }
    //默认的信号处理函数
    if(sig->sa_handler==SIG_DFL) {
        switch (signr) {
            // 如果信号是以下两个则也忽略之，并返回。
            case SIGCONT:
            case SIGCHLD:
                return (1);

            case SIGSTOP:
            case SIGTSTP:
            case SIGTTIN:
            case SIGTTOU:
                //挂起当前任务
                sysTasks.currentTask->status = TASK_SUSPEND;
                sysTasks.currentTask->exitCode = signr;
                if (!(sysTasks.currentTask->parentTask->signals[SIGCHLD-1].sa_flags &
                      SA_NOCLDSTOP)) {
                    //如果父进程不在停止状态，并且没有设置SA_NOCLDSTOP则发送SIGCHLD信号给父进程
                    sysTasks.currentTask->parentTask->signalBMap |= (1 << (SIGCHLD - 1));
                }
                return (1);  /* Reschedule another event */

                // 如果信号是以下6种信号之一，那么若信号产生了core dump，则以退出码为signr|0x80
                // 调用do_exit()退出。否则退出码就是信号值。do_exit()的参数是返回码和程序提供的退出
                // 状态信息。可作为wait()或waitpid()函数的状态信息。参见sys/wait.h文件第13-18行。
                // wait()或waitpid()利用这些宏就可以取得子进程的退出状态码或子进程终止的原因（信号）。
            case SIGQUIT:
            case SIGILL:
            case SIGTRAP:
            case SIGIOT:
            case SIGFPE:
            case SIGSEGV:
                //直接干掉进程
                //这里还应该进行core_dump
//                if (core_dump(signr))
                DoExit(-1, (signr) | 0x80);
                /* fall through */
            default:
                //直接干掉进程
                DoExit(-1, signr);
        }
    }

    uint32_t *_psp=psp;

    if(!(sig->sa_flags & SA_NOMASK)) {
        *(_psp) = 1;
        //不阻塞，则压入阻塞列表
        *(--_psp) = sysTasks.currentTask->signalBlocked;
        *(--_psp) = (uint32_t)0x01000000L; /* xPSR */
    }else{
        *(_psp) = 0;
        *(--_psp) = NULL;
        *(--_psp) = (uint32_t)0x01000000L; /* xPSR */
    }
    *(--_psp) = ((uint32_t)sig->sa_handler); /* Entry Point */
    /* R14 (LR) (init value will cause fault if ever used)*/
    *(--_psp) = (uint32_t)sig->sa_restorer;/*LR*/
    *(--_psp) = (uint32_t)0x12121212L; /* R12*/
    *(--_psp) = (uint32_t)0x03030303L; /* R3 */
    *(--_psp) = (uint32_t)0x02020202L; /* R2 */
    *(--_psp) = (uint32_t)0x01010101L; 	/* R1 */
    *(--_psp) = (uint32_t)signr; 				/* R0 : argument */

    //设置新的用户栈
    SetPSP(_psp);
    //如果只处理一次，则清空信号处理函数，设置为默认
    if(sig->sa_flags&SA_ONESHOT){
        sig->sa_handler=NULL;
    }
    //设置阻塞码
    sysTasks.currentTask->signalBlocked |= sig->sa_mask;
    //处理完成信号，复位信号
    sysTasks.currentTask->signalBMap&=~(1<<(signr-1));
    return 0;
}
/**
 * 信号返回处理
 * @param psp
 * @return
 */
int32_t sys_sigreturn(void* psp){
    uint32_t *_psp=psp;
    if(_psp[-10]==1) {
        sysTasks.currentTask->signalBlocked = _psp[-9];
    }
    void* newPSP=((uint32_t)psp)+9*4;
    SetPSP(newPSP);
    return 0;
}

void do_signal_isr(void* sp){
    if(sysTasks.currentTask->skInfo.stackType==1){
        //去掉阻塞的信号
        uint32_t bBmp=(~sysTasks.currentTask->signalBlocked) & sysTasks.currentTask->signalBMap;
        for(uint32_t i=0;i<_NSIG;i++) {
            //检查信号是否有效
            if (!(bBmp & (1 << i))) {
                continue;
            }
            //信号一个一个处理，处理完成则返回,下次到来在处理另外一个
            if (do_signal(sp ,i+1) == 1) {
                TaskSche();
            }
            return ;
        }
    }
}
