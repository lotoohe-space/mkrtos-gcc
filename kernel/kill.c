//
// Created by zz on 2022/2/14.
//
#include <sys/types.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
/**
 * 杀死某个进程
 * @param pid
 * @param sig
 */
int32_t  do_kill_pid(pid_t pid,int32_t sig){
   struct task* tmp;
   uint32_t t;
   t=DisCpuInter();
   tmp= find_task(pid);
   if(tmp==CUR_TASK && tmp->PID!=pid){
    return -ESRCH;
   }
   //发送kill信号
   if(sig) {
       inner_set_task_sig(tmp,sig);
   }
//   if(sig) {//发送指定信号
//       inner_set_sig(sig);
//   }
   RestoreCpuInter(t);
   return 0;
}
/**
 * 杀死某一组
 * @param pgid
 * @param sig
 * @return
 */
int32_t do_kill_group(pid_t pgid,int32_t sig){
    uint32_t t;
    struct task* tmp;
    t=DisCpuInter();
    tmp=sysTasks.allTaskList;
    while(tmp){
        if(pgid==tmp->PGID){
            if(sig) {
                inner_set_task_sig(tmp,sig);
            }
//            if(sig) {//发送指定信号
//                inner_set_sig(sig);
//            }
        }
        tmp=tmp->nextAll;
    }
    RestoreCpuInter(t);
    return 0;
}
/**
 * 杀死某个进程
 * @param pid
 * @param sig
 * @return
 */
int32_t sys_kill(pid_t pid,int32_t sig){
    int ret=0;
    if(sig<0||sig>NSIG){
        return -EINVAL;
    }
    if(pid>0){
        ret= do_kill_pid(pid,sig);
    }else if(pid==0){
        ret= do_kill_group(CUR_TASK->PGID,sig);
    }else if(pid==-1){

    }else if(pid<-1){

    }
    return ret;
}
