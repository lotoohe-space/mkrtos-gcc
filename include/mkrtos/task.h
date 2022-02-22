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
* @brief �����״̬
*/
typedef enum{

    /**
    * @brief	������Ϣ
    */
    TASK_REST=0,
    /**
    * @brief	��������
    */
    TASK_RUNNING,
    /**
    * @brief	�������
    */
    TASK_SUSPEND,
//    /**
//    * @brief	����ر���
//    */
//    TASK_CLOSING,
    /**
     * @brief �����Ѿ����ر���
     */
    TASK_CLOSED,
}TaskStatus;

/**
* @brief ������
*/
typedef void (*TaskFun)(void *arg0,void *arg1);


/**
* @brief �������������б�
*/
typedef struct _TaskCreatePar{
    /**
    * @brief ������
    */
    TaskFun taskFun;
    /**
    * @brief ����0
    */
    void *arg0;
    /**
    * @brief ����1
    */
    void *arg1;
    /**
    * @brief ���ȼ�
    */
    uint8_t prio;
    /**
    * @brief �û�ջ��С
    */
    uint16_t userStackSize;
    /**
    * @brief �ں�ջ��С
    */
    uint16_t kernelStackSize;
    /**
    * @brief ��������
    */
    const char* taskName;
    /**
     * @brief ���صĴ���
     */
    int32_t err;
}*PTaskCreatePar,TaskCreatePar;


/**
* @brief ջ����
*/
struct _stackInfo{
    /**
    * @brief ��ջ��ջ��ָ��
    */
    void *pspStack;
    void *mspStack;
    /**
    * @brief ʹ�õ���MSP����PSP 0ʹ��msp 1ʹ��psp
    */
    uint16_t stackType;
    /**
    * @brief svc�ж��Ƿ����,pendsv����Ƕ��svc�ж�,Ϊ1��������svc����
    */
    uint16_t svcStatus;
};



struct _SysTaskBaseLinks;
struct sigaction;
struct mem_struct;
struct tty_struct;
/**
* @brief	������ƿ�
*/
typedef struct task{
    /**
    * @brief ���ڵ�
    */
    struct _SysTaskBaseLinks *parent;
    /**
     * @brief ������
     */
    struct task *parentTask;
    /**
    * @brief ͬ���ȼ�����
    */
    struct task *next;
    /**
    * @brief �������������
    */
    struct task *nextAll;

    /**
     * @brief ɾ���ȴ�����
     */
    struct task *del_wait;
    /**
     * �رյȴ�����
     */
    struct wait_queue *close_wait;
    /**
    * @brief ��ջ��ջ��ָ�룬�������ս�ʱ�����ڴ��ͷ�
    */
    void *memLowStack;
    /**
    * @brief �û�ջ��С
    */
    uint32_t userStackSize;
    /**
    * @brief �ں�ջ��С
    */
    uint32_t kernelStackSize;
    /**
    * @brief �洢��ջ��Ϣ
    */
    struct _stackInfo skInfo;

    /**
    * @brief ����ʱ�����
    */
    uint32_t runCount;
    /**
    * @brief ����id
    */
    int32_t PID;
    /**
     * @brief ��ID
     */
    int32_t PGID;
    /**
    * @brief ��ǰ״̬
    */
    TaskStatus status;
    /**
    * @brief �������ȼ�
    */
    uint8_t prio;
    /**
    * @brief �������б�־,�� @BlockCon
    *0bit ��ʱ����
    *1bit ��������������
    *2bit �ź���pend����
    *3bit ��Ϣ��ȡ����
    */
    uint32_t flag;
    /**
     * @brief �źŵ�λͼ
     */
    uint32_t signalBMap;
    /**
     * @brief �źŴ���
     */
    struct sigaction signals[_NSIG];
    /**
     * @brief �ź�mask��
     */
    uint32_t sig_mask;

    /**
     * @brief ��ʱ����ms
     */
    uint32_t alarm;

    /**
    * @brief ��������
    */
    const char*	taskName;
    /**
     * @brief �˳���
     */
    int32_t exitCode;

    /**
     * ��������ڴ��������������ر�ʱ������������������ڴ潫���ͷ�
     */
    struct mem_struct *mems;

    uint8_t is_s_user;
    /**
     * ��ʵid
     */
    uid_t ruid;
    /**
     * ��Чid
     */
    uid_t euid;
    /**
     * �����id
     */
    uid_t suid;

    /**
     * ��ʵid
     */
    uid_t rgid;
    uid_t egid;//��Чid
    uid_t sgid;//�����id

    /**
     * �ڽ��̴���һ�����ļ�����Ŀ¼ʱ����һ����ʹ���ļ���ʽ���������� (����3 . 3��3 . 4�ڣ�
     * ����������˵���� o p e n��c r e a t��������������������һ������ m o d e����ָ�������ļ��Ĵ�ȡ
     * ���Ȩλ)�����ǽ���4 . 2 0��˵����δ���һ����Ŀ¼�����ļ���ʽ������������Ϊ 1��λ����
     * �ļ�m o d e�е���Ӧλ��һ����ת��0��
     */
    mode_t maak;
    struct tty_struct *tty;//��ǰ����ʹ�õ�tty������Ǻ�̨���̣���ò���ΪNULL
    struct file files[NR_FILE];//�ļ�
    void* root_inode; //��inode
    void* pwd_inode;//��ǰĿ¼

}*PTaskBlock,TaskBlock;




/**
* @brief ϵͳ������������洢��ͬ���ȼ�������ͷ
*/
typedef struct _SysTaskBaseLinks{
    /**
    * @brief ͬ���ȼ���������
    */
    PTaskBlock pSysTaskLinks;
    /**
    * @brief ��������ȼ�
    */
    uint8_t taskPriority;
    /**
    * @brief �������
    */
    uint16_t taskCount;
    /**
    * @brief �����������
    */
    uint16_t taskReadyCount;
    /**
    * @brief ��һ������ͷ�ڵ�
    */
    struct _SysTaskBaseLinks *next;

}*PSysTaskBaseLinks,SysTaskBaseLinks;

/**
* @brief ϵͳ����
*/
typedef struct{

    /**
    * @brief ϵͳ����
    */
    SysTaskBaseLinks sysTaskBaseLinks;
    /**
    * @brief ���б���������������
    */
    PTaskBlock pBlockedLinks;
    /**
    * @brief ����ʱ��
    */
    uint32_t sysRunCount;

    /**
    * @brief ��ǰ������ȼ�����ڵ�
    */
    PSysTaskBaseLinks currentMaxTaskNode;
    /**
    * @brief �Ƿ��״�,��Ϊ0������Ϊ1
    */
    uint8_t isFirst;
    /**
    * @brief ��ǰ����������ڵ�
    */
    PTaskBlock currentTask;
    /**
    * @brief ϵͳ������
    */
    uint32_t tasksCount;

    /**
    * @brief �Ƿ�����ϵͳ�����������
    */
    Atomic_t isSch;
    /**
    * @brief �������������
    */
    PTaskBlock allTaskList;
    /**
    * @brief ���������������IDʹ��
    */
    Atomic_t pidTemp;
}*PSysTasks,SysTasks;

extern SysTasks sysTasks;

struct task* find_task(int32_t PID);
void    task_sche(void);
int32_t add_task(struct task *add);
void    del_task(struct task** task_ls, struct task* del);
int32_t task_create(PTaskCreatePar tcp,void* progInfo);

//�ȴ�����
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
