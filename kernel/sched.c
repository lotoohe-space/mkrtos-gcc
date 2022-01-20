//
// Created by Administrator on 2022/1/9.
//

#include <type.h>
#include <arch/arch.h>
#include <mkrtos/task.h>
#include <mkrtos/mem.h>
#include <errno.h>
#include <string.h>
/**
* @brief ϵͳ�����
*/
SysTasks sysTasks={0};

/**
* @brief ͨ��PID�ҵ��������
*/
PTaskBlock find_task(int32_t PID){
    PTaskBlock pstl;
    if(PID<0){
        return sysTasks.currentTask;
    }
  //  uint32_t t=DisCpuInter();
    pstl=sysTasks.allTaskList;
    while(pstl){
        if(pstl->PID==PID){
         //   RestoreCpuInter(t);
            return pstl;
        }
        pstl=pstl->nextAll;
    }
  //  RestoreCpuInter(t);
    return NULL;
}
/**
* @brief ������ȣ����������ȹرգ��������Ч
*/
void task_sche(void){
    //����Ƿ���Ե���
    if(atomic_test(&(sysTasks.isSch),TRUE)){
        _TaskSchedule();
    }
}
/**
* @brief ���¾�����������ȼ�������ڵ㣬���µ������о�������������0
* @param pSysTasks ����������
*/
void update_cur_task(void){

    PSysTaskBaseLinks ptl;
    uint32_t t=DisCpuInter();

    ptl=sysTasks.sysTaskBaseLinks.next;

    /*������ȼ�*/
    PSysTaskBaseLinks maxPrioTask=NULL;

    while(ptl){
        if(ptl->taskReadyCount>0){
            if(maxPrioTask==NULL
               && ptl->taskReadyCount>0
                    ){
                maxPrioTask=ptl;
            }else{
                if(ptl->taskPriority > maxPrioTask->taskPriority
                    //	&& ptl->taskReadyCount>0
                        ){
                    /*����*/
                    maxPrioTask=ptl;
                }
            }
        }
        ptl=ptl->next;
    }
    sysTasks.currentMaxTaskNode=maxPrioTask;

    /*��ǰ�����Ľڵ�*/
    //pSysTasks->currentTask=NULL;

    RestoreCpuInter(t);
}

/**
* @brief ͨ�����ȼ��ҵ�������
* @param prio �������ȼ�
* @return ������ȼ�����ı�ͷ
*/
static PSysTaskBaseLinks FindTaskLinks(uint8_t prio){
    PSysTaskBaseLinks ptl;
    ptl=sysTasks.sysTaskBaseLinks.next;
    while(ptl){
        if(ptl->taskPriority == prio){
            return ptl;
        }
        ptl=ptl->next;
    }
    return NULL;
}
/**
* @brief ͨ�����ȼ��������ͷ�����ָ�����ȼ������ڣ��򴴽�һ�����ȼ��Ľڵ�
* @param prio ��ӵ����ȼ�
 * @return ��ӳɹ�������ͷ
*/
static PSysTaskBaseLinks AddLinks(uint8_t prio){

    PSysTaskBaseLinks pSysTaskBaseLinks
            =(PSysTaskBaseLinks)OSMalloc(sizeof(SysTaskBaseLinks));
    if(pSysTaskBaseLinks == NULL){
        return NULL;
    }
    pSysTaskBaseLinks->next=NULL;
    pSysTaskBaseLinks->pSysTaskLinks=NULL;
    pSysTaskBaseLinks->taskCount=0;
    pSysTaskBaseLinks->taskPriority=prio;
    pSysTaskBaseLinks->taskReadyCount=0;

    /*�������*/
    pSysTaskBaseLinks->next=sysTasks.sysTaskBaseLinks.next;
    sysTasks.sysTaskBaseLinks.next=pSysTaskBaseLinks;

    return pSysTaskBaseLinks;
}
/**
* @brief ͨ�����ȼ�����������������ȼ������ڣ��򴴽������ȼ�������ڵ�
* @param pSysTasks ����������
* @return ����Ƿ�ɹ�
*/
int32_t add_task(PTaskBlock pTaskBlock){

    if(pTaskBlock==NULL){
        return -1;
    }
    /*�������ж�*/
    uint32_t t;
    t=DisCpuInter();
//	/*��ס�����رյ���*/
//	TaskSetIsSchedule(FALSE);
//	SpinLock(&sysTasks.slh);

    PSysTaskBaseLinks taskLinks;

    taskLinks = FindTaskLinks(pTaskBlock->prio);
    if(taskLinks == NULL){
        taskLinks=AddLinks(pTaskBlock->prio);
        if(taskLinks==NULL){
            RestoreCpuInter(t);
//			TaskSetIsSchedule(TRUE);
//			SpinUnlock(&sysTasks.slh);
            errno=ENOMEM;
            /*û���ڴ�*/
            return -1;
        }
    }
    taskLinks->taskCount++;
    taskLinks->taskReadyCount++;

    //�ŵ�ͬ���ȼ�������������
    PTaskBlock pstl=taskLinks->pSysTaskLinks;
    if(pstl == NULL){
        taskLinks->pSysTaskLinks=pTaskBlock;
    }else{
        /*���������ͷ*/
        pTaskBlock->next=pstl;
        taskLinks->pSysTaskLinks=pTaskBlock;
    }
    pTaskBlock->parent=taskLinks;

    //�浽���������������
    pstl=sysTasks.allTaskList;
    if(pstl == NULL){
        sysTasks.allTaskList=pTaskBlock;
    }else{
        /*���������ͷ*/
        pTaskBlock->nextAll=pstl;
        sysTasks.allTaskList=pTaskBlock;
    }
//
//	TaskSetIsSchedule(TRUE);
//	SpinUnlock(&sysTasks.slh);
    RestoreCpuInter(t);

    //�������ȼ�
    if(
            sysTasks.currentMaxTaskNode!=NULL
            &&
            pTaskBlock->prio>sysTasks.currentMaxTaskNode->taskPriority
            ) {
        update_cur_task();
    }else if(sysTasks.currentMaxTaskNode==NULL){
        update_cur_task();
    }
    return 0;
}

/**
* @brief ��������м��
*/
void tasks_check(void){
    PTaskBlock ptb;
    /*�������ж�*/
   // uint32_t t=DisCpuInter();
    sysTasks.sysRunCount++;

    //������еĶ�ʱ���ź�
    ptb=sysTasks.allTaskList;
    while(ptb){
        if(ptb->alarm){
            //ʱ�䵽��
            if(ptb->alarm<sysTasks.sysRunCount){
                ptb->signalBMap|=1<<(SIGALRM-1);
                ptb->alarm = 0;
            }
        }
        //����ʱ��+1
        ptb->runCount++;
nextAll:
        ptb=ptb->nextAll;
    }

}


/**
* @brief ִ��������ȣ�������һ�������ջ��
* @brief SP ��ǰ�̵߳�ջ����ַ
* @brief SPType ʹ�õ���MSP����PSP 0ʹ��msp 1ʹ��psp
* @return ����ջ��
*/
struct _stackInfo* sys_task_sche(void* psp,void* msp,uint32_t spType){
    uint16_t svcStatus=FALSE;
    volatile uint32_t *svcPendReg=(uint32_t*)(0xE000ED24);
    //����svc״̬
    if((*svcPendReg)&0x80L){
        (*svcPendReg)&=~(0x80L);
        svcStatus=TRUE;
    }
    if(sysTasks.isFirst==FALSE){
        /*��һ��δ���䣬�����һ��*/
        PTaskBlock ptb=sysTasks.currentMaxTaskNode->pSysTaskLinks;
        while(ptb){
            if(ptb->status!=TASK_RUNNING
                //|| ptb->delayCount>0
                    ){
                ptb=ptb->next;
                continue;
            }

            sysTasks.currentTask=ptb;
            sysTasks.isFirst=TRUE;
            break;
        }
    }else{
        /*�޸ĵ�ǰ�̵߳�ջ����ַ*/
        sysTasks.currentTask->skInfo.pspStack=psp;
        sysTasks.currentTask->skInfo.mspStack=msp;
        if(sysTasks.currentTask->skInfo.stackType!=2){
            sysTasks.currentTask->skInfo.stackType=spType;
        }
        sysTasks.currentTask->skInfo.svcStatus=svcStatus;
        /*֮ǰ�������ֱ������һ����Ч��*/
        PTaskBlock ptb;
        ptb= sysTasks.currentTask->next;
        do{
            if(ptb == NULL){
                ptb = sysTasks.currentMaxTaskNode->pSysTaskLinks;
            }
            if(ptb->status==TASK_RUNNING
                /*|| ptb->delayCount>0*/
                    ){
                sysTasks.currentTask=ptb;
                break;
            }

            ptb=ptb->next;
        }while(1);
    }
    //�ָ������svc״̬
    if(sysTasks.currentTask->skInfo.svcStatus==1){
        (*svcPendReg)|=0x80L;
        (*svcPendReg)&=~(0x100L);
    }
    /*���ض�ջ�ĵ�ַ*/
    return &(sysTasks.currentTask->skInfo);
}

// ϵͳ���ù��� - ���ñ�����ʱʱ��ֵ(��)��
// ����Ѿ����ù�alarm ֵ���򷵻ؾ�ֵ�����򷵻�0��
int32_t
sys_alarm (uint32_t seconds){
    uint32_t old = 0;
    if(sysTasks.currentTask->alarm){
        old=sysTasks.currentTask->alarm;
    }
    sysTasks.currentTask->alarm = sysTasks.sysRunCount+(seconds*1000)/(1000/OS_WORK_HZ);
    return old;
}



//���Ѷ��������е�����
void wake_up(struct wait_queue *queue){
    uint32_t t;
    t=DisCpuInter();
    while(queue){
        if(queue->task){
            if(queue->task->status==TASK_SUSPEND){
                queue->task->status=TASK_RUNNING;
            }
        }
        queue=queue->next;
    }
    RestoreCpuInter(t);
}

//���һ�����ȴ�������
void add_wait_queue(struct wait_queue ** queue,struct wait_queue* add_queue){
    uint32_t t;
    t=DisCpuInter();
    if(*queue==NULL){
        *queue=add_queue;
    }else{
       add_queue->next = (*queue);
       *queue=add_queue;
    }
    RestoreCpuInter(t);
}
//�Ƴ�һ���ȴ���
void remove_wait_queue(struct wait_queue ** queue,struct wait_queue* add_queue){
    struct wait_queue *temp=*queue;
    struct wait_queue *prev=NULL;
    while(temp){
        if(temp==add_queue) {
            if (prev==NULL) {
                //ɾ���ĵ�һ��
                *queue=temp->next;
                break;
            }else{
                prev->next=temp->next;
                break;
            }
        }
        prev=temp;
        temp=temp->next;
    }
}


// ȡ��ǰ���̺�pid��
int32_t
sys_getpid (void)
{
    return sysTasks.currentTask->PID;
}

// ȡ�����̺�ppid��
int32_t
sys_getppid (void)
{
    return -ENOSYS;
}

// ȡ�û���uid��
int32_t
sys_getuid (void)
{
    return -ENOSYS;
}

// ȡeuid��
int32_t
sys_geteuid (void)
{
    return -ENOSYS;
}

// ȡ���gid��
int32_t
sys_getgid (void)
{
    return -ENOSYS;
}

// ȡegid��
int32_t
sys_getegid (void)
{
    return -ENOSYS;
}

/**
 * @brief ������ǰ�������ȼ�
 * @param increment
 * @return
 */
int32_t
sys_nice (int32_t increment)
{
    if (sysTasks.currentTask->prio - increment > 0)
        sysTasks.currentTask->prio -= increment;
    return 0;
}
extern void KernelTaskInit(void);
// ���ȳ���ĳ�ʼ���ӳ���
void SchedInit(void)
{
    InitMem();
    /*OS�Ƿ���ȳ�ʼ��*/
    atomic_set(&sysTasks.isSch,1);
    /*����pid�������*/
    atomic_set(&sysTasks.pidTemp,1);
    sysTasks.currentMaxTaskNode=NULL;
    sysTasks.tasksCount=0;
    KernelTaskInit();
}
