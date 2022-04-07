//
// Created by Administrator on 2022/1/9.
//
#include <mkrtos/mem.h>
#include <type.h>
#include <mkrtos/task.h>
#include "arch/arch.h"
#include <string.h>
#include <sched.h>
extern PTaskBlock find_task(int32_t PID);
extern int lwip_fork(int s) ;
//pipe.c
void do_fork_pipe(struct inode *inode,struct task* new_task,int fd);

// fn为函数指针，此指针指向一个函数体，即想要创建进程的静态程序（我们知道进程的4要素，这个就是指向程序的指针，就是所谓的“剧本", ）；
//     child_stack为给子进程分配系统堆栈的指针（在linux下系统堆栈空间是2页面，就是8K的内存，其中在这块内存中，低地址上放入了值，这个值就是进程控制块task_struct的值）；
//      arg就是传给子进程的参数一般为（0）；
//     flags为要复制资源的标志，描述你需要从父进程继承那些资源（是资源复制还是共享，在这里设置参数：
//下面是flags可以取的值
//  标志                    含义
//  CLONE_PARENT   创建的子进程的父进程是调用者的父进程，新进程与创建它的进程成了“兄弟”而不是“父子”
//  CLONE_FS           子进程与父进程共享相同的文件系统，包括root、当前目录、umask
//  CLONE_FILES      子进程与父进程共享相同的文件描述符（file descriptor）表
//  CLONE_NEWNS   在新的namespace启动子进程，namespace描述了进程的文件hierarchy
//  CLONE_SIGHAND   子进程与父进程共享相同的信号处理（signal handler）表
//  CLONE_PTRACE   若父进程被trace，子进程也被trace 不支持
//  CLONE_VFORK     父进程被挂起，直至子进程释放虚拟内存资源
//  CLONE_VM           子进程与父进程运行于相同的内存空间
//  CLONE_PID          子进程在创建时PID与父进程一致
//  CLONE_THREAD    Linux 2.4中增加以支持POSIX线程标准，子进程与父进程共享相同的线程群

//child_stack是用户栈，内核栈还是需要申请
int32_t sys_clone(int (*fn)(void*),void* child_stack,int flags,void* arg){

    if(!fn //|| !child_stack
    ) {
        return -EINVAL;
    }

    uint32_t t=DisCpuInter();
    PTaskBlock ptb=CUR_TASK;
    PTaskBlock newPtb=OSMalloc(sizeof(TaskBlock));
    if(newPtb==NULL){
        RestoreCpuInter(t);
        return -1;
    }

    memcpy(newPtb,ptb,sizeof(TaskBlock));
    newPtb->status=TASK_SUSPEND;
    newPtb->runCount=0;
    newPtb->next=NULL;
    newPtb->nextAll=NULL;

    newPtb->memLowStack=NULL;

    //申请内核栈空间
    newPtb->knl_low_stack= (void *) OSMalloc(sizeof(uint32_t) * (newPtb->kernelStackSize));
    if (newPtb->knl_low_stack == NULL) {
        OSFree(newPtb);
        RestoreCpuInter(t);
        return -1;
    }

    if(!(flags&CLONE_PID)) {
        newPtb->PID = (pid_t) atomic_read(&sysTasks.pidTemp);
    }

    //clone内存空间
    if (ptb->exec) {
        void *exec_tmp = newPtb->exec;
        //重新复制应用信息
        newPtb->exec = OSMalloc(sizeof(ELFExec_t));
        if (!newPtb->exec) {
            OSFree(newPtb);
            OSFree( newPtb->knl_low_stack);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->exec, exec_tmp, sizeof(ELFExec_t));

        if(!(flags&CLONE_VM)) {
            //不使用相同的内存空间，RAM都是独立的
            newPtb->exec->data.data = OSMalloc(ptb->exec->data.sh_size);
            if (!newPtb->exec->data.data) {
                OSFree(newPtb);
                OSFree(ptb->exec);
                OSFree( newPtb->knl_low_stack);
                RestoreCpuInter(t);
                return -1;
            }
            memcpy(newPtb->exec->data.data, ptb->exec->data.data, ptb->exec->data.sh_size);
            newPtb->exec->bss.data = OSMalloc(ptb->exec->bss.sh_size);
            if (!newPtb->exec->bss.data) {
                OSFree(newPtb);
                OSFree(ptb->exec);
                OSFree(newPtb->exec->data.data);
                OSFree( newPtb->knl_low_stack);
                RestoreCpuInter(t);
                return -1;
            }
            memcpy(newPtb->exec->bss.data, ptb->exec->bss.data, ptb->exec->bss.sh_size);
//            newPtb->exec->clone_vm=TRUE;
        }
        //标志该exec块使用次数+1
        (*(ptb->exec->used_count))++;
    }

    int32_t err;
    /*通过优先级添加任务*/
    err = add_task(newPtb,1);
    if(err != 0){
        //	RestoreCpuInter(t);
        /*释放申请的内存*/
        OSFree(newPtb);
        OSFree(ptb->exec);
        OSFree( newPtb->knl_low_stack);
        OSFree(newPtb->exec->data.data);
        OSFree(newPtb->exec->bss.data);
        RestoreCpuInter(t);
        return -1;
    }

    atomic_inc(&sysTasks.pidTemp);

    //设置内核栈位置
    newPtb->skInfo.mspStack=(void*)(&(((uint32_t*)newPtb->knl_low_stack)[ptb->kernelStackSize-1]));
    newPtb->skInfo.mspStack = OSTaskSetReg(newPtb->skInfo.mspStack, fn, arg, 0, 0);
    //设置用户栈
    newPtb->skInfo.pspStack=child_stack;
    if(newPtb->skInfo.pspStack) {
        //重新定位
        newPtb->skInfo.pspStack = OSTaskSetReg(newPtb->skInfo.pspStack, fn, arg, 0, 0);
    }else{
        newPtb->userStackSize=0;
        newPtb->skInfo.pspStack=(void*)(~(0L));
    }
    if(!newPtb->userStackSize){
        newPtb->skInfo.stackType=2;
    }else{
        newPtb->skInfo.stackType=1;
    }
    //设置为用户模式
    newPtb->skInfo.svcStatus=0;
    if(flags&CLONE_PARENT){
        //CLONE_PARENT   创建的子进程的父进程是调用者的父进程，新进程与创建它的进程成了“兄弟”而不是“父子”
        newPtb->parentTask=ptb->parentTask;
    }else{
        //设置父进程
        newPtb->parentTask=ptb;
    }

    if(!(flags&CLONE_FS)){
        //不使用父进程的FS信息
        //用0号进程的吧
        newPtb->root_inode=sysTasks.init_task->root_inode;
        newPtb->pwd_inode=sysTasks.init_task->pwd_inode;
        //引用计数器+1
        atomic_inc(&newPtb->root_inode->i_used_count);
        atomic_inc(&newPtb->pwd_inode->i_used_count);
        newPtb->mask=0;
        newPtb->pwd_path[0]='\0';
    }

    if(flags&CLONE_FILES) {
        //子进程与父进程共享相同的文件描述符（file descriptor）表
        //对于打开的文件，我们应当对其inode的引用进行+1的操作。
        for (int i = 0; i < NR_FILE; i++) {
            if (newPtb->files[i].used) {
                struct inode *tmp;
                if(newPtb->files[i].net_file){
                    lwip_fork(newPtb->files[i].net_sock);
                }else {
                    tmp = newPtb->files[i].f_inode;
                    if (tmp) {
                        if (i < 3) {
                            tmp->i_ops->default_file_ops->open(tmp, &newPtb->files[i]);
                        }
                        //对pipe进行fork
                        do_fork_pipe(tmp, newPtb, i);
                        atomic_inc(&(tmp->i_used_count));
                    }
                }
            }
        }
    }else{
        //不复制父进程的file
        //前三个不用管
        for (int i = 0; i < NR_FILE; i++) {
            if(i<3){
                if(newPtb->files[i].used) {
//                    if(newPtb->files[i].net_file){
//                        newPtb->files[i].sock_used_cn++;
//                    }else {
                        if (newPtb->files[i].f_inode
                                ) {
                            atomic_inc(&newPtb->files[i].f_inode->i_used_count);
                        }
//                    }
                }
            }else {
                if (newPtb->files[i].used) {
                    newPtb->files[i].used = 0;
                }
            }
        }
    }
    newPtb->del_wait=NULL;
    newPtb->close_wait=NULL;

    if(!(flags&CLONE_SIGHAND)) {
        //不共享信号处理表
        newPtb->sig_bmp[0] = 0;
        newPtb->sig_bmp[1] = 0;
        for (int i = 0; i < _NSIG; i++) {
            newPtb->signals[i]._u._sa_handler = SIG_DFL;
        }
    }
    if(flags&CLONE_THREAD){
        newPtb->parentTask=ptb->parentTask;
        newPtb->tpid=ptb->PID;
    }

    //mem信息不进行fork
    newPtb->mems=NULL;
    newPtb->status=TASK_RUNNING;
    newPtb->clone_flag=flags;
    RestoreCpuInter(t);

    //返回pid
    return newPtb->PID;
}

//创建一个子进程
int32_t sys_fork(uint32_t *psp){
//    uint32_t t=DisCpuInter();
    PTaskBlock ptb=CUR_TASK;
    PTaskBlock newPtb=OSMalloc(sizeof(TaskBlock));
    if(newPtb==NULL){
//        RestoreCpuInter(t);
        return -1;
    }
    memcpy(newPtb,ptb,sizeof(TaskBlock));
    newPtb->status=TASK_SUSPEND;
    newPtb->runCount=0;
    newPtb->next=NULL;
    newPtb->nextAll=NULL;
    newPtb->memLowStack=(void *)OSMalloc(sizeof(uint32_t)*(newPtb->userStackSize));
    if(newPtb->memLowStack==NULL){
        OSFree(newPtb);
//        RestoreCpuInter(t);
        return -1;
    }
    newPtb->knl_low_stack=(void *)OSMalloc(sizeof(uint32_t)*(newPtb->kernelStackSize));
    if(newPtb->knl_low_stack==NULL){
        OSFree(newPtb->memLowStack);
        OSFree(newPtb);
//        RestoreCpuInter(t);
        return -1;
    }
    newPtb->PID = (pid_t)atomic_read(&sysTasks.pidTemp);
    uint32_t t=DisCpuInter();
    if(ptb->exec){
        void* exec_tmp=newPtb->exec;
        //重新复制应用信息
        newPtb->exec=OSMalloc(sizeof(ELFExec_t));
        if(!newPtb->exec){
            OSFree(newPtb);
            OSFree(newPtb->memLowStack);
            OSFree(newPtb->knl_low_stack);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->exec,exec_tmp,sizeof(ELFExec_t));
        newPtb->exec->data.data = OSMalloc(ptb->exec->data.sh_size);
        if(!newPtb->exec->data.data){
            OSFree(newPtb);
            OSFree(ptb->exec);
            OSFree(newPtb->memLowStack);
            OSFree(newPtb->knl_low_stack);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->exec->data.data,ptb->exec->data.data,ptb->exec->data.sh_size);
        newPtb->exec->bss.data = OSMalloc(ptb->exec->bss.sh_size);
        if(!newPtb->exec->bss.data){
            OSFree(newPtb);
            OSFree(ptb->exec);
            OSFree(newPtb->memLowStack);
            OSFree(newPtb->knl_low_stack);
            OSFree(newPtb->exec->data.data);
            RestoreCpuInter(t);
            return -1;
        }
        memcpy(newPtb->exec->bss.data,ptb->exec->bss.data,ptb->exec->bss.sh_size);
        (*(ptb->exec->used_count))++;
    }

    int32_t err;
    newPtb->parent=NULL;
    /*通过优先级添加任务*/
    err = add_task(newPtb,1);
    if(err != 0){
        //	RestoreCpuInter(t);
        /*释放申请的内存*/
        OSFree(newPtb);
        OSFree(ptb->exec);
        OSFree(newPtb->memLowStack);
        OSFree(newPtb->knl_low_stack);
        OSFree(newPtb->exec->data.data);
        OSFree(newPtb->exec->bss.data);
        RestoreCpuInter(t);
        return -1;
    }

    atomic_inc(&sysTasks.pidTemp);
    //设置父进程
    newPtb->parentTask=ptb;
    //复制栈
    memcpy(newPtb->memLowStack,ptb->memLowStack,sizeof(uint32_t)*(newPtb->userStackSize));
    memcpy(newPtb->knl_low_stack,ptb->knl_low_stack,sizeof(uint32_t)*(newPtb->kernelStackSize));

    //设置栈位置
    newPtb->skInfo.mspStack=(void*)(&(((uint32_t*)newPtb->knl_low_stack)[ptb->kernelStackSize-1]));
    if(newPtb->userStackSize!=0){
        newPtb->skInfo.pspStack=(void*)((uint32_t)(newPtb->memLowStack)+((uint32_t)(psp)- (uint32_t)(ptb->memLowStack)));
        ((uint32_t*)(newPtb->skInfo.pspStack))[8]=0;
    }else{
        newPtb->skInfo.pspStack=(void*)(~(0L));
    }
    RestoreCpuInter(t);
    //设置为用户模式
    newPtb->skInfo.svcStatus=0;
    newPtb->skInfo.stackType=1;

    //对于打开的文件，我们应当对其inode的引用进行+1的操作。
    for(int i=0;i<NR_FILE;i++){
        if(newPtb->files[i].used){
            struct inode *tmp;
            if(newPtb->files[i].net_file){
                lwip_fork(newPtb->files[i].net_sock);
            }else {
                tmp = newPtb->files[i].f_inode;
                if (tmp) {
                    if (i < 3) {
                        tmp->i_ops->default_file_ops->open(tmp, &newPtb->files[i]);
                    }
                    //对pipe进行fork
                    do_fork_pipe(tmp, newPtb, i);
                    atomic_inc(&(tmp->i_used_count));
                }
            }
        }
    }
    //引用计数器+1
    if(newPtb->root_inode) {
        atomic_inc(&newPtb->root_inode->i_used_count);
    }
    if(newPtb->pwd_inode) {
        atomic_inc(&newPtb->pwd_inode->i_used_count);
    }
    newPtb->del_wait=NULL;
    newPtb->close_wait=NULL;
    newPtb->sig_bmp[0]=0;
    newPtb->sig_bmp[1]=0;
    newPtb->sig_mask[0]=0;
    newPtb->sig_mask[1]=0;
    for(int i=0;i<_NSIG;i++) {
        newPtb->signals[i]._u._sa_handler=SIG_DFL;
    }
    //mem信息不进行fork
    newPtb->mems=NULL;
    newPtb->status=TASK_RUNNING;


    //返回pid
    return newPtb->PID;
}