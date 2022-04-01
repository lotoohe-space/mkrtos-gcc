//
// Created by Administrator on 2022/1/9.
//

#ifndef UNTITLED1_TASK_H
#define UNTITLED1_TASK_H
#include "type.h"
#include "arch/atomic.h"
#include "mkrtos/signal.h"
#include "ipc/spin_lock.h"
#include <mkrtos/fs.h>
#include <loader.h>
#define NR_FILE 8

#define CUR_TASK sysTasks.currentTask

//struct file;

/**
* @brief 任务的状态
*/
typedef enum{

    TASK_RUNNING,//任务运行
    TASK_SUSPEND,//任务挂起
    TASK_CLOSED,// 任务已经被关闭了
    TASK_UNINTR,//任务不可被中断
}TaskStatus;

/**
* @brief 任务函数
*/
typedef void (*TaskFun)(void *arg0,void *arg1);


/**
* @brief 函数创建传参列表
*/
typedef struct _TaskCreatePar{
    /**
    * @brief 任务函数
    */
    TaskFun taskFun;
    /**
    * @brief 参数0
    */
    void *arg0;
    /**
    * @brief 参数1
    */
    void *arg1;
    /**
    * @brief 优先级
    */
    uint8_t prio;
    /**
    * @brief 用户栈大小
    */
    uint16_t userStackSize;
    /**
    * @brief 内核栈大小
    */
    uint16_t kernelStackSize;
    /**
    * @brief 任务名字
    */
    const char* taskName;
    /**
     * @brief 返回的错误
     */
    int32_t err;
}*PTaskCreatePar,TaskCreatePar;


/**
* @brief 栈类型
*/
struct _stackInfo{
    /**
    * @brief 堆栈的栈顶指针
    */
    void *pspStack;
    void *mspStack;
    /**
    * @brief 使用的是MSP还是PSP 0使用msp 1使用psp 2内核线程模式
    */
    uint16_t stackType;
    /**
    * @brief svc中断是否产生,pendsv可能嵌套svc中断,为1代表正在svc调用
    */
    uint16_t svcStatus;
};



struct _SysTaskBaseLinks;
struct sigaction;
struct mem_struct;
struct tty_struct;
/**
* @brief	任务控制块
*/
typedef struct task{
    struct _SysTaskBaseLinks *parent;//父节点
    struct task *parentTask;//父进程
    struct task *next;//同优先级链表
    struct task *nextAll;//所有任务的链表
    struct task *del_wait;//删除等待队列
    struct wait_queue *close_wait;//关闭等待队列

    void *memLowStack;//用戶堆栈的栈低指针，当任务终结时用于内存释放
    void *knl_low_stack;//内核栈
    uint32_t userStackSize;//用户栈大小
    uint32_t kernelStackSize;//内核栈大小
    struct _stackInfo skInfo;//存储堆栈信息

    uint32_t runCount;//运行时间计数
    TaskStatus status;//当前状态
    uint8_t prio;//任务优先级
    const char*	taskName;// 任务名称

    uint32_t sig_bmp[_NSIG_WORDS];//信号的位图
    struct sigaction signals[_NSIG];//信号处理
    uint32_t sig_mask[_NSIG_WORDS];//信号mask

    uint32_t alarm;//定时多少ms


    int32_t exitCode;//退出码
    struct mem_struct *mems;//管理进程内存申请的链表，任务关闭时，链表中所有申请的内存将被释放

    uint8_t is_s_user; //是否超级用户

    pid_t PID;//进程id
    pid_t tpid;//线程的pid

    uid_t ruid;//真实user id
    uid_t euid;//有效user id
    uid_t suid;//保存的user id
    uid_t rgid;//真实group id
    int32_t PGID; //组ID
    uid_t egid;//有效组id
    uid_t sgid;//保存的组id


    int clone_flag;//克隆标志

    /**
     * 在进程创建一个新文件或新目录时，就一定会使用文件方式创建屏蔽字 (回忆3 . 3和3 . 4节，
     * 在那里我们说明了 o p e n和c r e a t函数。这两个函数都有一个参数 m o d e，它指定了新文件的存取
     * 许可权位)。我们将在4 . 2 0节说明如何创建一个新目录，在文件方式创建屏蔽字中为 1的位，在
     * 文件m o d e中的相应位则一定被转成0。
     */
    mode_t mask;//创建文件时设置的mask
    struct tty_struct *tty;//当前进程使用的tty，如果是后台进程，则该参数为NULL
    struct file files[NR_FILE];//文件
    struct inode* root_inode; //根inode
    struct inode* pwd_inode;//当前目录
    char pwd_path[128];//当前的绝对路径
    ELFExec_t *exec;//当前应用的上下文
}*PTaskBlock,TaskBlock;




/**
* @brief 系统任务基础链表，存储不同优先级的链表头
*/
typedef struct _SysTaskBaseLinks{
    /**
    * @brief 同优先级的任务链
    */
    PTaskBlock pSysTaskLinks;
    /**
    * @brief 任务的优先级
    */
    uint8_t taskPriority;
    /**
    * @brief 任务个数
    */
    uint16_t taskCount;
    /**
    * @brief 就绪任务个数
    */
    uint16_t taskReadyCount;
    /**
    * @brief 下一个任务头节点
    */
    struct _SysTaskBaseLinks *next;

}*PSysTaskBaseLinks,SysTaskBaseLinks;

/**
* @brief 系统任务
*/
typedef struct{

    /**
    * @brief 系统任务
    */
    SysTaskBaseLinks sysTaskBaseLinks;
    /**
    * @brief 所有被阻塞的任务链表
    */
    struct task* pBlockedLinks;
    struct task* init_task;
    /**
    * @brief 运行时间
    */
    uint32_t sysRunCount;

    /**
    * @brief 当前最高优先级任务节点
    */
    PSysTaskBaseLinks currentMaxTaskNode;
    /**
    * @brief 是否首次,是为0，不是为1
    */
    uint8_t isFirst;
    /**
    * @brief 当前工作的任务节点
    */
    struct task* currentTask;
    /**
    * @brief 系统任务数
    */
    uint32_t tasksCount;

    /**
    * @brief 是否允许系统进行任务调度
    */
    Atomic_t isSch;
    /**
    * @brief 所有任务的链表
    */
    PTaskBlock allTaskList;
    /**
    * @brief 创建任务分配任务ID使用
    */
    Atomic_t pidTemp;
}*PSysTasks,SysTasks;

extern SysTasks sysTasks;

struct task* find_task(int32_t PID);
void    task_sche(void);
int32_t add_task(struct task *add,uint32_t into_all_ls);
void    del_task(struct task** task_ls, struct task* del,int flag);
int32_t task_create(PTaskCreatePar tcp);

//等待链表
struct wait_queue{
    struct task* task;
    struct wait_queue *next;
};
//sched.c
void wake_up(struct wait_queue *queue);
void add_wait_queue(struct wait_queue ** queue,struct wait_queue* add_queue);
struct wait_queue * find_wait_queue(struct wait_queue ** queue, struct task* tk,uint32_t *max_prio);
void remove_wait_queue(struct wait_queue ** queue,struct wait_queue* add_queue);
int32_t task_change_prio(struct task *tk,int32_t new_prio);
void update_cur_task(void);
void task_suspend(void);
void task_unintr(void);
void task_run(void);
void task_run_1(struct task* tk);
int32_t sys_pause(void);

//printk.c
void printk(const char *fmt, ...);
void fatalk(const char* fmt, ...);

//signal.c
int32_t inner_set_sig(uint32_t signum);
int32_t inner_set_task_sig(struct task *tk,uint32_t signum);
int32_t inner_set_pid_sig(pid_t pid,uint32_t signum);

//exit.c
void pre_exit(void);

//sleep.c
struct sleep_time_queue{
    struct task* task;
    struct sleep_time_queue *next;
    uint32_t slp_ms;
    uint32_t cur_ms;
};
void _do_check_sleep_tim(struct sleep_time_queue **slp_ls);
void do_check_sleep_tim(void);
void do_remove_sleep_tim(struct task* tk) ;
void add_sleep(struct sleep_time_queue **queue,struct sleep_time_queue *add);
void remove_sleep(struct sleep_time_queue **queue,struct sleep_time_queue *rm);
int do_nanosleep(struct sleep_time_queue **slp_ls,const struct timespec *req,struct timespec *rem,Atomic_t *lock,int check_val
        );
int sys_nanosleep(const struct timespec *req, struct timespec *rem);
void wake_up_sleep(struct sleep_time_queue **queue);
#endif //UNTITLED1_TASK_H
