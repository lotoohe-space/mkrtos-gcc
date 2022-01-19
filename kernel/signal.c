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
 * ��������linux 0.11
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
// sigaction()ϵͳ���á��ı�������յ�һ���ź�ʱ�Ĳ�����signum�ǳ���SIGKILL�����
// �κ��źš�[����²�����action����Ϊ�� ]���²�������װ����� oldactionָ�벻Ϊ�գ�
// ��ԭ������������oldaction���ɹ��򷵻�0������Ϊ-EINVAL��
int sys_sigaction(int signum, const struct sigaction * action,
                  struct sigaction * oldaction)
{

    if (signum<1 || signum>32 || signum==SIGKILL || signum==SIGSTOP)
        return -EINVAL;

    sysTasks.currentTask->signals[signum-1]=*action;
    if (oldaction)
        *oldaction = sysTasks.currentTask->signals[signum-1];
    // ��������ź����Լ����źž�����յ�������������Ϊ0�������������α��źš�
    if (sysTasks.currentTask->signals[signum-1].sa_flags & SA_NOMASK)
        sysTasks.currentTask->signals[signum-1].sa_mask = 0;
    else
        sysTasks.currentTask->signals[signum-1].sa_mask |= (1<<(signum-1));
    return 0;
}
/**
 * ���ﴦ���ź�
 * @param psp
 * @param signr
 * @return
 */
int32_t do_signal(void* psp,uint32_t signr){
    struct sigaction  *sig=&(sysTasks.currentTask->signals[signr - 1]);
    //�����ź�
    if(sig->sa_handler==SIG_IGN){
        return -1;
    }
    //Ĭ�ϵ��źŴ�����
    if(sig->sa_handler==SIG_DFL) {
        switch (signr) {
            // ����ź�������������Ҳ����֮�������ء�
            case SIGCONT:
            case SIGCHLD:
                return (1);

            case SIGSTOP:
            case SIGTSTP:
            case SIGTTIN:
            case SIGTTOU:
                //����ǰ����
                sysTasks.currentTask->status = TASK_SUSPEND;
                sysTasks.currentTask->exitCode = signr;
                if (!(sysTasks.currentTask->parentTask->signals[SIGCHLD-1].sa_flags &
                      SA_NOCLDSTOP)) {
                    //��������̲���ֹͣ״̬������û������SA_NOCLDSTOP����SIGCHLD�źŸ�������
                    sysTasks.currentTask->parentTask->signalBMap |= (1 << (SIGCHLD - 1));
                }
                return (1);  /* Reschedule another event */

                // ����ź�������6���ź�֮һ����ô���źŲ�����core dump�������˳���Ϊsignr|0x80
                // ����do_exit()�˳��������˳�������ź�ֵ��do_exit()�Ĳ����Ƿ�����ͳ����ṩ���˳�
                // ״̬��Ϣ������Ϊwait()��waitpid()������״̬��Ϣ���μ�sys/wait.h�ļ���13-18�С�
                // wait()��waitpid()������Щ��Ϳ���ȡ���ӽ��̵��˳�״̬����ӽ�����ֹ��ԭ���źţ���
            case SIGQUIT:
            case SIGILL:
            case SIGTRAP:
            case SIGIOT:
            case SIGFPE:
            case SIGSEGV:
                //ֱ�Ӹɵ�����
                //���ﻹӦ�ý���core_dump
//                if (core_dump(signr))
                DoExit(-1, (signr) | 0x80);
                /* fall through */
            default:
                //ֱ�Ӹɵ�����
                DoExit(-1, signr);
        }
    }

    uint32_t *_psp=psp;

    if(!(sig->sa_flags & SA_NOMASK)) {
        *(_psp) = 1;
        //����������ѹ�������б�
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

    //�����µ��û�ջ
    SetPSP(_psp);
    //���ֻ����һ�Σ�������źŴ�����������ΪĬ��
    if(sig->sa_flags&SA_ONESHOT){
        sig->sa_handler=NULL;
    }
    //����������
    sysTasks.currentTask->signalBlocked |= sig->sa_mask;
    //��������źţ���λ�ź�
    sysTasks.currentTask->signalBMap&=~(1<<(signr-1));
    return 0;
}
/**
 * �źŷ��ش���
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
        //ȥ���������ź�
        uint32_t bBmp=(~sysTasks.currentTask->signalBlocked) & sysTasks.currentTask->signalBMap;
        for(uint32_t i=0;i<_NSIG;i++) {
            //����ź��Ƿ���Ч
            if (!(bBmp & (1 << i))) {
                continue;
            }
            //�ź�һ��һ��������������򷵻�,�´ε����ڴ�������һ��
            if (do_signal(sp ,i+1) == 1) {
                TaskSche();
            }
            return ;
        }
    }
}
