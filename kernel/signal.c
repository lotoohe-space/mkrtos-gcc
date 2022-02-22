//
// Created by Administrator on 2022/1/9.
//
#include "arch/arch.h"
#include <type.h>
#include <mkrtos/task.h>
#include <errno.h>
#include <signal.h>
void DoExit(int32_t exitCode);

//发送SIGCHLD给父进程
void sig_chld(struct task *tk){
    struct sigaction *sig;
    tk=CUR_TASK->parentTask;
    sig=&(tk->parentTask->signals[SIGCHLD-1]);
    if(sig->sa_handler!=SIG_IGN){
        //如果设置了SA_NOCLDSTOP标志，则不返送SIGCHLD信号给
        if(sig->sa_flags&SA_NOCLDSTOP){
            return ;
        }
        tk->signalBMap|=(1<<(SIGCHLD-1));
    }
}
//int32_t inner_set_sig(struct task* tk,uint32_t signum){
//    if (signum<1 || signum>32){
//        return -EINVAL;
//    }
//
//    //设置相应的位
//    tk->signalBMap|=(1<<(signum-1));
//    return 0;
//}

int32_t sys_sgetmask()
{
    return CUR_TASK->sig_mask;
}

int32_t sys_ssetmask(int32_t newmask)
{
    int32_t old=CUR_TASK->sig_mask;

    CUR_TASK->sig_mask = newmask & ~(1<<(SIGKILL-1)) & ~(1<<(SIGSTOP-1));
    return old;
}
extern void rt_sigreturn();
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
            ) {
        return -EINVAL;
    }

    temp.sa_handler = (void (*)(int)) handler;
    temp.sa_mask.sig[0] = 0;
    temp.sa_mask.sig[1] = 0;
    temp.sa_flags = SA_RESETHAND | SA_NODEFER;
//    temp.sa_restorer = (void (*)(void)) restorer;
    temp.sa_restorer=rt_sigreturn;//直接指向一个系统调用
    handler = (uint32_t) CUR_TASK->signals[signum-1].sa_handler;
    CUR_TASK->signals[signum-1] = temp;
    return handler;
}
// 如果参数act不是一个空指针，它将指向一个结构，指定与指定信号相关的动作。
// 与指定信号相关联的结构。
// 如果参数oact不是一个空指针，那么之前与信号相关的动作 被存储在参数oact所指向的位置。
// 如果 参数act是一个空指针，信号的处理是不变的；
// 因此，该调用可用于 查询当前对一个给定信号的处理。
// SIGKILL和SIGSTOP信号应 不应使用这种机制将SIGKILL和SIGSTOP信号添加到信号掩码中；
// 这一限制应被系统强制执行。这个限制应该由系统来执行，而不会导致显示错误。

//如果在sigaction结构的sa_flags字段中清除了SA_SIGINFO标志（见下文），
// sa_handler字段就会确定与指定信号相关的动作。
// 如果sa_flags字段中的SA_SIGINFO标志被设置，并且实现支持实时信号扩展选项或XSI扩展选项，
// sa_sigaction字段指定了一个信号捕捉函数。
//sa_flags字段可用于修改指定信号的行为。

int sys_sigaction(int sig, const struct sigaction *restrict act,struct sigaction *restrict oact){

    if (sig<1 || sig>32 || sig==SIGKILL || sig==SIGSTOP){
        return -EINVAL;
    }
    if(!act){
        return -EINVAL;
    }
    if(oact){
        *oact = CUR_TASK->signals[sig-1];
    }

    CUR_TASK->signals[sig-1]=*act;
    CUR_TASK->signals[sig-1].sa_restorer= (void (*)(void)) rt_sigreturn;
    // 如果允许信号在自己的信号句柄中收到，则令屏蔽码为0，否则设置屏蔽本信号。
    if (CUR_TASK->signals[sig-1].sa_flags & SA_NODEFER) {
        CUR_TASK->signals[sig - 1].sa_mask.sig[0]=0;
        CUR_TASK->signals[sig - 1].sa_mask.sig[1]=0;
    }else {
        CUR_TASK->signals[sig - 1].sa_mask.sig[0] |= (1 << (sig - 1));
    }
//    if(CUR_TASK->signals[sig-1].sa_flags&SA_RESETHAND){
//        CUR_TASK->signals[sig - 1].sa_mask.sig[0]=0;
//        CUR_TASK->signals[sig - 1].sa_mask.sig[1]=0;
//    }
    return 0;
}
int sys_rt_sigaction(int signum, const struct sigaction * action,
                     struct sigaction * oldaction){
    return sys_sigaction(signum,action,oldaction);
}

/**
 * 这里处理信号，信号处理是在中断调用结束后处理的，信号不能够在
 * @param signr
 * @return
 */
int32_t do_signal(struct task* tk,void *cur_psp,uint32_t signr){
    struct sigaction  *sig;
    if(
        tk->skInfo.svcStatus==1
        ||tk->userStackSize==0
        ){
        // 任务必须没有正在执行系统调用
        // 内核线程不能够执行信号量
        return -2;
    }
    sig = &(tk->signals[signr - 1]);
    //忽略信号
    if(sig->sa_handler==SIG_IGN){
        return -1;
    }
    //默认的信号处理函数
    if(sig->sa_handler==SIG_DFL) {
        switch (signr) {
            // 如果信号是以下两个则也忽略之，并返回。
            case SIGCONT:
                //此作业控制信号送给需要继续运行的处于停止状态的进程。
                // 如果接收到此信号的进程处于停止状态，
                // 则系统默认动作是使该进程继续运行，否则默认动作是忽略此信号。
                if(tk->status==TASK_SUSPEND){
                    tk->status=TASK_RUNNING;
                }
            case SIGCHLD:
               return 1;

            case SIGSTOP:
            case SIGTSTP:
            case SIGTTIN:
            case SIGTTOU:
                //挂起当前任务
                tk->status = TASK_SUSPEND;
                tk->exitCode = signr;
                sig_chld(tk);
//                if (!(tk->parentTask->signals[SIGCHLD-1].sa_flags &
//                      SA_NOCLDSTOP)) {
//                    tk->parentTask->signalBMap |= (1 << (SIGCHLD - 1));
//                }
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
                DoExit( (signr) | 0x80);
                return 0;
            default:
                //直接干掉进程
                DoExit( signr);
                return 0;
        }
    }

    if(sig->sa_flags&SA_ONSTACK){
        //是否采用备用栈

    }
    uint32_t *_psp;
    if(tk==CUR_TASK){
        _psp = cur_psp;
    }else {
        _psp = tk->skInfo.pspStack;
    }
    //
//    if(!(sig->sa_flags & SA_NODEFER)) {
//        *(_psp) = ;
        //不阻塞，则压入阻塞列表
        *(_psp) = sig->sa_mask.sig[0];
        *(--_psp) = (uint32_t)0x01000000L; /* xPSR */
//    }else{
//        *(_psp) = 0;
//        *(--_psp) = NULL;
//        *(--_psp) = (uint32_t)0x01000000L; /* xPSR */
//    }
    *(--_psp) = ((uint32_t)sig->sa_handler); /* Entry Point */
    /* R14 (LR) (init value will cause fault if ever used)*/
    *(--_psp) = (uint32_t)sig->sa_restorer;/*LR*/
    *(--_psp) = (uint32_t)0x12121212L; /* R12*/
    *(--_psp) = (uint32_t)0x03030303L; /* R3 */
    if(sig->sa_flags&SA_SIGINFO){
        struct siginfo siginfo;
        siginfo.si_signo=signr;
        siginfo.si_errno=0;
        siginfo.si_code=0;
        *(--_psp) = (uint32_t) NULL; /* R2 */
        *(--_psp) = (uint32_t) &siginfo;    /* R1 */
        *(--_psp) = (uint32_t) signr;        /* R0 : argument */
    }else {
        *(--_psp) = (uint32_t) 0x02020202L; /* R2 */
        *(--_psp) = (uint32_t) 0x01010101L;    /* R1 */
        *(--_psp) = (uint32_t) signr;        /* R0 : argument */
    }

    //设置新的用户栈
    SetPSP(_psp);
    //如果只处理一次，则清空信号处理函数，设置为默认
    if(sig->sa_flags&SA_RESETHAND){
        sig->sa_handler=SIG_DFL;
        //恢复默认值
        sig->sa_flags=0;
    }
    CUR_TASK->sig_mask|=sig->sa_mask.sig[0];
//    if(!(sig->sa_flags&SA_NODEFER)) {
//        //在处理信号时不在收到该信号
//        sysTasks.currentTask->sig_mask |= (1<<(signr-1));
//    }
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
    //针对SA_NODEFER标志，复位sig_mask
//    sysTasks.currentTask->sig_mask &= ~(1<<(_psp[8]-1));
    CUR_TASK->sig_mask&=~(_psp[9]);
    void* newPSP=((uint32_t)psp)+9*4;
    SetPSP(newPSP);
    return 0;
}
int32_t sys_rt_sigreturn(void* psp){
    return sys_sigreturn(psp);
}
//#define __NR_rt_sigreturn		(__NR_SYSCALL_BASE+173)
//#define __NR_rt_sigaction		(__NR_SYSCALL_BASE+174)
//#define __NR_rt_sigprocmask		(__NR_SYSCALL_BASE+175)
//#define __NR_rt_sigpending		(__NR_SYSCALL_BASE+176)
//#define __NR_rt_sigtimedwait		(__NR_SYSCALL_BASE+177)
//#define __NR_rt_sigqueueinfo		(__NR_SYSCALL_BASE+178)
//#define __NR_rt_sigsuspend		(__NR_SYSCALL_BASE+179)
void do_signal_isr_(struct task* tk,void *cur_psp){
//    void *sp;
//    获得用户栈
//    sp=tk->skInfo.pspStack;
    if(tk->status!=TASK_CLOSED){
        //过滤掉阻塞的
        uint32_t bBmp=(~tk->sig_mask) & tk->signalBMap;
        if(!bBmp) {
            return ;
        }
        for (uint32_t i = 0; i < _NSIG; i++) {
            //检查信号是否有效
            if (!(bBmp & (1 << i))) {
                continue;
            }
            //信号一个一个处理，处理完成则返回,下次到来在处理另外一个
            if (do_signal(tk,cur_psp, i + 1) == 1) {
                task_sche();
            }
        }
    }
}
void do_signal_isr(void* sp){
    //在该函数中检测每一个没有被关闭的任务的信号
    struct task* tmp;
//    uint32_t t;
//    t=DisCpuInter();
    tmp=sysTasks.allTaskList;
    while(tmp){
        //如果是当前进程
        do_signal_isr_(tmp,sp);
        tmp=tmp->nextAll;
    }
//    RestoreCpuInter(t);
}
