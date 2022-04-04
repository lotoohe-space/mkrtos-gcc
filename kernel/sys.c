//
// Created by Administrator on 2022/1/9.
//

#include <errno.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
#include <sys/utsname.h>
#include <string.h>
#include <sys/times.h>
#include <sys/resource.h>

char* sys_getenv(char* key){
    return NULL;
}
int sys_vhangup(void){
    return -ENOSYS;
}
// 废弃了
int sys_ftime ()
{
    return -ENOSYS;
}

//废弃了
int sys_break ()
{
    return -ENOSYS;
}

// 用于当前进程对子进程进行调试(degugging)。
int sys_ptrace (int request,int pid,int addr,int data)
{
    return -ENOSYS;
}

// 改变并打印终端行设置。
int sys_stty ()
{
    return -ENOSYS;
}

// 取终端行设置信息。
int sys_gtty ()
{
    return -ENOSYS;
}

//
int sys_prof ()
{
    return -ENOSYS;
}
int sys_setregid (int rgid, int egid){
    return -ENOSYS;
}
// 设置进程组号(gid)。如果任务没有超级用户特权，它可以使用setgid()将其有效gid
// （effective gid）设置为成其保留gid(saved gid)或其实际gid(real gid)。如果任务有
// 超级用户特权，则实际gid、有效gid 和保留gid 都被设置成参数指定的gid。
int sys_setgid (gid_t gid)
{
    if(CUR_TASK->is_s_user){
        CUR_TASK->egid=gid;
        CUR_TASK->rgid=gid;
        CUR_TASK->sgid=gid;
    }else {
        if(gid==CUR_TASK->rgid
           ||gid==CUR_TASK->sgid
                ){
            CUR_TASK->egid=gid;
        }else{
            return -EPERM;
        }
    }
    return 0;
}

// 打开或关闭进程计帐功能。
int sys_acct ()
{
    return -ENOSYS;
}

// 映射任意物理内存到进程的虚拟地址空间。
int sys_phys ()
{
    return -ENOSYS;
}

int sys_lock ()
{
    return -ENOSYS;
}

int sys_mpx ()
{
    return -ENOSYS;
}

int sys_ulimit ()
{
    return -ENOSYS;
}
extern uint32_t rtc_set_tick(uint32_t tick);
extern uint32_t rtc_get_tick(void);
extern uint32_t rtc_get_msofsec(void);
// 返回从1970 年1 月1 日00:00:00 GMT 开始计时的时间值（秒）。如果tloc 不为null，则时间值
// 也存储在那里。
int sys_time (time_t *tloc)
{
    time_t tick;
    //后面应该使用rtc驱动设备，但是这样是最快的rtc.c

    tick=rtc_get_tick();
    if(tloc){
        *tloc=tick;
    }
    return tick;
}
int sys_setreuid (uid_t ruid, uid_t euid){

    if(ruid!=-1){
        CUR_TASK->ruid=ruid;
    }
    if(euid!=-1){
        CUR_TASK->euid=euid;
    }

    return -ENOSYS;
}
//设置用户id
int sys_setuid (uid_t uid) {

    if(CUR_TASK->is_s_user){
        CUR_TASK->euid=uid;
        CUR_TASK->ruid=uid;
        CUR_TASK->suid=uid;
    }else {
        if(uid==CUR_TASK->ruid
        ||uid==CUR_TASK->suid
        ){
            CUR_TASK->euid=uid;
        }else{
            return -EPERM;
        }
    }
    return 0;
}
int sys_stime (time_t *tptr)
{
    if(!tptr){
        return -EINVAL;
    }
    if(!CUR_TASK->is_s_user){
        return -EPERM;
    }
    //rtc.c
    extern uint32_t rtc_set_tick(uint32_t tick);
    rtc_set_tick(*tptr);
    return -ENOSYS;
}
// 获取当前任务时间。tms 结构中包括用户时间、系统时间、子进程用户时间、子进程系统时间。
int sys_times (struct tms *tbuf)
{
    if(!tbuf){
        return -EINVAL;
    }
    tbuf->tms_utime=CUR_TASK->runCount;
    //下面三个后面再说吧，现在好像没啥用
    tbuf->tms_cstime=0;
    tbuf->tms_cutime=0;
    tbuf->tms_stime=0;
    return -ENOSYS;
}
int sys_settimeofday(struct timeval *tv, struct timezone *tz){
    if(!tv){
        return -EINVAL;
    }
    if(!CUR_TASK->is_s_user){
        return -EPERM;
    }

    rtc_set_tick(tv->tv_sec+tv->tv_usec/1000/1000);

    return 0;
}
int sys_gettimeofday(struct timeval *tv, struct timezone *tz){

    if(!tv){
        return -EINVAL;
    }
//    struct tm *tm;
    time_t r_tim;
    r_tim=rtc_get_tick();
//    tm=gmtime(&r_tim);
    tv->tv_sec=r_tim;
    tv->tv_usec=rtc_get_msofsec()*1000;
//    DST_NONE     /* not on DST */
//    DST_USA      /* USA style DST */
//    DST_AUST     /* Australian style DST */
//    DST_WET      /* Western European DST */
//    DST_MET      /* Middle European DST */
//    DST_EET      /* Eastern European DST */
//    DST_CAN      /* Canada */
//    DST_GB       /* Great Britain and Eire */
//    DST_RUM      /* Romania */
//    DST_TUR      /* Turkey */
//    DST_AUSTALT  /* Australian style with shift in 1986 */
    if(tz){
        tz->tz_dsttime=0;
        tz->tz_minuteswest=-480;
    }

    return -ENOSYS;
}
// 当参数end_data_seg 数值合理，并且系统确实有足够的内存，而且进程没有超越其最大数据段大小
// 时，该函数设置数据段末尾为end_data_seg 指定的值。该值必须大于代码结尾并且要小于堆栈
// 结尾16KB。返回值是数据段的新结尾值（如果返回值与要求值不同，则表明有错发生）。
// 该函数并不被用户直接调用，而由libc 库函数进行包装，并且返回值也不一样。
int sys_brk (unsigned long end_data_seg)
{
    //没有虚拟内存的概念，直接返回成功
    return 0;
}
/**
 * 设置组ID
 * @param pid
 * @param pgid
 * @return
 */
int32_t sys_setpgid (int32_t pid, int32_t pgid){
    uint32_t t;
    struct task* md_task;
    t=DisCpuInter();
    md_task = find_task(pid);
    if(!pgid){
        md_task->PGID=md_task->PID;
    }else {
        md_task->PGID = pgid;
    }
    RestoreCpuInter(t);
    return -ENOSYS;
}
int sys_getpgrp (void)
{
    return -ENOSYS;
}
int sys_setsid (void){
    return -ENOSYS;
}

//没想好放到哪儿，先放到这里吧
#define SYS_NAME "mkrtos"
#define SYS_VERSION "0.01"
#define SYS_RELEASE "0.01"
#define SYS_DOMAINNAME SYS_NAME
#define SYS_MACHINE "cortex-m3 open"

int sys_uname (struct utsname *name) {
    if(!name){
        return -1;
    }
    strncpy(name->sysname,SYS_NAME,_UTSNAME_LENGTH);
    strncpy(name->version,SYS_VERSION,_UTSNAME_LENGTH);
    strncpy(name->release,SYS_RELEASE,_UTSNAME_LENGTH);
    strncpy(name->domainname,SYS_NAME,_UTSNAME_DOMAIN_LENGTH);
    strncpy(name->machine,SYS_MACHINE,_UTSNAME_LENGTH);
    strncpy(name->nodename,SYS_NAME,_UTSNAME_NODENAME_LENGTH);
    return -ENOSYS;
}
/**
 * 在进程创建一个新文件或新目录时，就一定会使用文件方式创建屏蔽字 (回忆3 . 3和3 . 4节，
 * 在那里我们说明了 o p e n和c r e a t函数。这两个函数都有一个参数 m o d e，它指定了新文件的存取
 * 许可权位)。我们将在4 . 2 0节说明如何创建一个新目录，在文件方式创建屏蔽字中为 1的位，在
 * 文件m o d e中的相应位则一定被转成0。
    */
mode_t sys_umask (mode_t mask){
    mode_t old_mask;
    old_mask=CUR_TASK->mask;
    CUR_TASK->mask=mask;
    return old_mask;
}

int sys_seteuid(uid_t uid){
    if(CUR_TASK->is_s_user){
        CUR_TASK->euid=uid;
    }else {
        if(uid==CUR_TASK->ruid||uid==CUR_TASK->suid){
            CUR_TASK->euid=uid;
        }else{
            return -EPERM;
        }
    }
    return 0;
}
int sys_setegid(gid_t gid){
    if(CUR_TASK->is_s_user){
        CUR_TASK->egid=gid;
    }else {
        if(gid==CUR_TASK->ruid||gid==CUR_TASK->egid){
            CUR_TASK->euid=gid;
        }else{
            return -EPERM;
        }
    }
    return 0;
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
    if(CUR_TASK->parentTask){
        return CUR_TASK->parentTask->PID;
    }else{
        return -1;
    }
}

// 取用户号uid。
int32_t
sys_getuid (void)
{
    return CUR_TASK->ruid;
}

// 取euid。
int32_t
sys_geteuid (void)
{
    return CUR_TASK->euid;
}

// 取组号gid。
int32_t
sys_getgid (void)
{
    return CUR_TASK->rgid;
}

// 取egid。
int32_t
sys_getegid (void)
{
    return CUR_TASK->egid;
}
int32_t sys_getpriority(int which,int who){
    uint32_t t;
    int32_t ret=-ESRCH;
    struct task* tk;
    t=DisCpuInter();
    switch (which) {
        case PRIO_PROCESS:
            tk = find_task(who);
            ret=tk->prio;
            goto end;
//            tk->prio=who;
        case PRIO_PGRP:
            ret=-ENOSYS;
            goto end;
        case PRIO_USER:
            ret=-ENOSYS;
            goto end;
        default:
            RestoreCpuInter(t);
            return -EINVAL;
    }
        end:
    RestoreCpuInter(t);
    return ret;
}
int sys_setpriority(int which,int who, int prio){
    uint32_t t;
    int32_t ret=0;
    struct task* tk;
    if(!CUR_TASK->is_s_user){
        return -EPERM;
    }
    t=DisCpuInter();
    switch (which) {
        case PRIO_PROCESS:
            tk = find_task(who);
            if(task_change_prio(tk,prio)<=0){
                ret=-ESRCH;
            }
            goto end;
//            tk->prio=who;
        case PRIO_PGRP:
            //TODO:
            ret=-ENOSYS;
            goto end;
        case PRIO_USER:
            //TODO:
            ret=-ENOSYS;
            goto end;
        default:
            RestoreCpuInter(t);
            return -EINVAL;
    }
    end:
    RestoreCpuInter(t);
    return ret;
}



