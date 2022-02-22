//
// Created by Administrator on 2022/1/9.
//

#ifndef UNTITLED1_TASK_H
#define UNTITLED1_TASK_H
#include "type.h"
#include "arch/atomic.h"
#include "mkrtos/signal.h"
#include <mkrtos/fs.h>
#define NR_FILE 8

#define CUR_TASK sysTasks.currentTask

//struct file;

/**
* @brief 任务的状态
*/
typedef enum{

    /**
    * @brief	任务休息
    */
    TASK_REST=0,
    /**
    * @brief	任务运行
    */
    TASK_RUNNING,
    /**
    * @brief	任务挂起
    */
    TASK_SUSPEND,
//    /**
//    * @brief	任务关闭中
//    */
//    TASK_CLOSING,
    /**
     * @brief 任务已经被关闭了
     */
    TASK_CLOSED,
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
    * @brief 使用的是MSP还是PSP 0使用msp 1使用psp
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
    /**
    * @brief 父节点
    */
    struct _SysTaskBaseLinks *parent;
    /**
     * @brief 父进程
     */
    struct task *parentTask;
    /**
    * @brief 同优先级链表
    */
    struct task *next;
    /**
    * @brief 所有任务的链表
    */
    struct task *nextAll;

    /**
     * @brief 删除等待队列
     */
    struct task *del_wait;
    /**
     * 关闭等待队列
     */
    struct wait_queue *close_wait;
    /**
    * @brief 堆栈的栈低指针，当任务终结时用于内存释放
    */
    void *memLowStack;
    /**
    * @brief 用户栈大小
    */
    uint32_t userStackSize;
    /**
    * @brief 内核栈大小
    */
    uint32_t kernelStackSize;
    /**
    * @brief 存储堆栈信息
    */
    struct _stackInfo skInfo;

    /**
    * @brief 运行时间计数
    */
    uint32_t runCount;
    /**
    * @brief 进程id
    */
    int32_t PID;
    /**
     * @brief 组ID
     */
    int32_t PGID;
    /**
    * @brief 当前状态
    */
    TaskStatus status;
    /**
    * @brief 任务优先级
    */
    uint8_t prio;
    /**
    * @brief 任务运行标志,见 @BlockCon
    *0bit 延时阻塞
    *1bit 互斥锁加锁阻塞
    *2bit 信号量pend阻塞
    *3bit 消息获取阻塞
    */
    uint32_t flag;
    /**
     * @brief 信号的位图
     */
    uint32_t signalBMap;
    /**
     * @brief 信号处理
     */
    struct sigaction signals[_NSIG];
    /**
     * @brief 信号mask，
     */
    uint32_t sig_mask;

    /**
     * @brief 定时多少ms
     */
    uint32_t alarm;

    /**
    * @brief 任务名称
    */
    const char*	taskName;
    /**
     * @brief 退出码
     */
    int32_t exitCode;

    /**
     * 管理进程内存申请的链表，任务关闭时，链表中所有申请的内存将被释放
     */
    struct mem_struct *mems;

    uint8_t is_s_user;
    /**
     * 真实id
     */
    uid_t ruid;
    /**
     * 有效id
     */
    uid_t euid;
    /**
     * 保存的id
     */
    uid_t suid;

    /**
     * 真实id
     */
    uid_t rgid;
    uid_t egid;//有效id
    uid_t sgid;//保存的id

    /**
     * 在进程创建一个新文件或新目录时，就一定会使用文件方式创建屏蔽字 (回忆3 . 3和3 . 4节，
     * 在那里我们说明了 o p e n和c r e a t函数。这两个函数都有一个参数 m o d e，它指定了新文件的存取
     * 许可权位)。我们将在4 . 2 0节说明如何创建一个新目录，在文件方式创建屏蔽字中为 1的位，在
     * 文件m o d e中的相应位则一定被转成0。
     */
    mode_t maak;
    struct tty_struct *tty;//当前进程使用的tty，如果是后台进程，则该参数为NULL
    struct file files[NR_FILE];//文件
    void* root_inode; //根inode
    void* pwd_inode;//当前目录

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
    PTaskBlock pBlockedLinks;
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
    PTaskBlock currentTask;
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
int32_t add_task(struct task *add);
void    del_task(struct task** task_ls, struct task* del);
int32_t task_create(PTaskCreatePar tcp,void* progInfo);

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

//printk.c
void printk(const char *fmt, ...);
void fatalk(const char* fmt, ...);

#endif //UNTITLED1_TASK_H
