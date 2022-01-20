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
* @brief 系统任务表
*/
SysTasks sysTasks={0};

/**
* @brief 通过PID找到任务对象
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
* @brief 任务调度，如果任务调度关闭，则调用无效
*/
void task_sche(void){
    //监测是否可以调度
    if(atomic_test(&(sysTasks.isSch),TRUE)){
        _TaskSchedule();
    }
}
/**
* @brief 更新就绪的最高优先级到任务节点，更新的链表中就绪任务必须大于0
* @param pSysTasks 任务管理对象
*/
void update_cur_task(void){

    PSysTaskBaseLinks ptl;
    uint32_t t=DisCpuInter();

    ptl=sysTasks.sysTaskBaseLinks.next;

    /*最大优先级*/
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
                    /*更新*/
                    maxPrioTask=ptl;
                }
            }
        }
        ptl=ptl->next;
    }
    sysTasks.currentMaxTaskNode=maxPrioTask;

    /*当前工作的节点*/
    //pSysTasks->currentTask=NULL;

    RestoreCpuInter(t);
}

/**
* @brief 通过优先级找到任务链
* @param prio 任务优先级
* @return 最大优先级任务的表头
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
* @brief 通过优先级添加任务头，如果指定优先级不存在，则创建一个优先级的节点
* @param prio 添加的优先级
 * @return 添加成功的链表头
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

    /*链表接上*/
    pSysTaskBaseLinks->next=sysTasks.sysTaskBaseLinks.next;
    sysTasks.sysTaskBaseLinks.next=pSysTaskBaseLinks;

    return pSysTaskBaseLinks;
}
/**
* @brief 通过优先级添加任务，如果这个优先级不存在，则创建该优先级的任务节点
* @param pSysTasks 任务管理对象
* @return 添加是否成功
*/
int32_t add_task(PTaskBlock pTaskBlock){

    if(pTaskBlock==NULL){
        return -1;
    }
    /*关所有中断*/
    uint32_t t;
    t=DisCpuInter();
//	/*锁住链表并关闭调度*/
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
            /*没有内存*/
            return -1;
        }
    }
    taskLinks->taskCount++;
    taskLinks->taskReadyCount++;

    //放到同优先级任务链表里面
    PTaskBlock pstl=taskLinks->pSysTaskLinks;
    if(pstl == NULL){
        taskLinks->pSysTaskLinks=pTaskBlock;
    }else{
        /*放在链表最开头*/
        pTaskBlock->next=pstl;
        taskLinks->pSysTaskLinks=pTaskBlock;
    }
    pTaskBlock->parent=taskLinks;

    //存到所有任务的链表中
    pstl=sysTasks.allTaskList;
    if(pstl == NULL){
        sysTasks.allTaskList=pTaskBlock;
    }else{
        /*放在链表最开头*/
        pTaskBlock->nextAll=pstl;
        sysTasks.allTaskList=pTaskBlock;
    }
//
//	TaskSetIsSchedule(TRUE);
//	SpinUnlock(&sysTasks.slh);
    RestoreCpuInter(t);

    //更新优先级
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
* @brief 对任务进行检查
*/
void tasks_check(void){
    PTaskBlock ptb;
    /*关所有中断*/
   // uint32_t t=DisCpuInter();
    sysTasks.sysRunCount++;

    //检测所有的定时器信号
    ptb=sysTasks.allTaskList;
    while(ptb){
        if(ptb->alarm){
            //时间到了
            if(ptb->alarm<sysTasks.sysRunCount){
                ptb->signalBMap|=1<<(SIGALRM-1);
                ptb->alarm = 0;
            }
        }
        //运行时间+1
        ptb->runCount++;
nextAll:
        ptb=ptb->nextAll;
    }

}


/**
* @brief 执行任务调度，返回下一个任务的栈顶
* @brief SP 当前线程的栈顶地址
* @brief SPType 使用的是MSP还是PSP 0使用msp 1使用psp
* @return 返回栈顶
*/
struct _stackInfo* sys_task_sche(void* psp,void* msp,uint32_t spType){
    uint16_t svcStatus=FALSE;
    volatile uint32_t *svcPendReg=(uint32_t*)(0xE000ED24);
    //保存svc状态
    if((*svcPendReg)&0x80L){
        (*svcPendReg)&=~(0x80L);
        svcStatus=TRUE;
    }
    if(sysTasks.isFirst==FALSE){
        /*第一次未分配，则分配一个*/
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
        /*修改当前线程的栈顶地址*/
        sysTasks.currentTask->skInfo.pspStack=psp;
        sysTasks.currentTask->skInfo.mspStack=msp;
        if(sysTasks.currentTask->skInfo.stackType!=2){
            sysTasks.currentTask->skInfo.stackType=spType;
        }
        sysTasks.currentTask->skInfo.svcStatus=svcStatus;
        /*之前分配过，直接找下一个有效的*/
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
    //恢复任务的svc状态
    if(sysTasks.currentTask->skInfo.svcStatus==1){
        (*svcPendReg)|=0x80L;
        (*svcPendReg)&=~(0x100L);
    }
    /*返回堆栈的地址*/
    return &(sysTasks.currentTask->skInfo);
}

// 系统调用功能 - 设置报警定时时间值(秒)。
// 如果已经设置过alarm 值，则返回旧值，否则返回0。
int32_t
sys_alarm (uint32_t seconds){
    uint32_t old = 0;
    if(sysTasks.currentTask->alarm){
        old=sysTasks.currentTask->alarm;
    }
    sysTasks.currentTask->alarm = sysTasks.sysRunCount+(seconds*1000)/(1000/OS_WORK_HZ);
    return old;
}



//唤醒队列中所有的任务
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

//添加一个到等待队列中
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
//移除一个等待的
void remove_wait_queue(struct wait_queue ** queue,struct wait_queue* add_queue){
    struct wait_queue *temp=*queue;
    struct wait_queue *prev=NULL;
    while(temp){
        if(temp==add_queue) {
            if (prev==NULL) {
                //删除的第一个
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


// 取当前进程号pid。
int32_t
sys_getpid (void)
{
    return sysTasks.currentTask->PID;
}

// 取父进程号ppid。
int32_t
sys_getppid (void)
{
    return -ENOSYS;
}

// 取用户号uid。
int32_t
sys_getuid (void)
{
    return -ENOSYS;
}

// 取euid。
int32_t
sys_geteuid (void)
{
    return -ENOSYS;
}

// 取组号gid。
int32_t
sys_getgid (void)
{
    return -ENOSYS;
}

// 取egid。
int32_t
sys_getegid (void)
{
    return -ENOSYS;
}

/**
 * @brief 调整当前任务优先级
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
// 调度程序的初始化子程序。
void SchedInit(void)
{
    InitMem();
    /*OS是否调度初始化*/
    atomic_set(&sysTasks.isSch,1);
    /*进程pid分配变量*/
    atomic_set(&sysTasks.pidTemp,1);
    sysTasks.currentMaxTaskNode=NULL;
    sysTasks.tasksCount=0;
    KernelTaskInit();
}
