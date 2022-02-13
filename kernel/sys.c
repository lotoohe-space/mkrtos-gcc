//
// Created by Administrator on 2022/1/9.
//

#include <errno.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
// 返回日期和时间。
int
sys_ftime ()
{
    return -ENOSYS;
}

//
int
sys_break ()
{
    return -ENOSYS;
}

// 用于当前进程对子进程进行调试(degugging)。
int
sys_ptrace ()
{
    return -ENOSYS;
}

// 改变并打印终端行设置。
int
sys_stty ()
{
    return -ENOSYS;
}

// 取终端行设置信息。
int
sys_gtty ()
{
    return -ENOSYS;
}

//
int
sys_prof ()
{
    return -ENOSYS;
}
int
sys_setregid (int rgid, int egid){
    return -ENOSYS;
}
// 设置进程组号(gid)。如果任务没有超级用户特权，它可以使用setgid()将其有效gid
// （effective gid）设置为成其保留gid(saved gid)或其实际gid(real gid)。如果任务有
// 超级用户特权，则实际gid、有效gid 和保留gid 都被设置成参数指定的gid。
int
sys_setgid (int gid)
{
    return (sys_setregid (gid, gid));
}

// 打开或关闭进程计帐功能。
int
sys_acct ()
{
    return -ENOSYS;
}

// 映射任意物理内存到进程的虚拟地址空间。
int
sys_phys ()
{
    return -ENOSYS;
}

int
sys_lock ()
{
    return -ENOSYS;
}

int
sys_mpx ()
{
    return -ENOSYS;
}

int
sys_ulimit ()
{
    return -ENOSYS;
}
// 返回从1970 年1 月1 日00:00:00 GMT 开始计时的时间值（秒）。如果tloc 不为null，则时间值
// 也存储在那里。
int
sys_time (long *tloc)
{
    int i=0;


    return i;
}
int
sys_setreuid (int ruid, int euid){
    return -ENOSYS;
}
int
sys_setuid (int uid)
{
    return (sys_setreuid (uid, uid));
}
int
sys_stime (long *tptr)
{
    return -ENOSYS;
}
// 获取当前任务时间。tms 结构中包括用户时间、系统时间、子进程用户时间、子进程系统时间。
int
sys_times (struct tms *tbuf)
{
    return -ENOSYS;
}
// 当参数end_data_seg 数值合理，并且系统确实有足够的内存，而且进程没有超越其最大数据段大小
// 时，该函数设置数据段末尾为end_data_seg 指定的值。该值必须大于代码结尾并且要小于堆栈
// 结尾16KB。返回值是数据段的新结尾值（如果返回值与要求值不同，则表明有错发生）。
// 该函数并不被用户直接调用，而由libc 库函数进行包装，并且返回值也不一样。
int
sys_brk (unsigned long end_data_seg)
{
    return -ENOSYS;
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
    md_task->PGID=pgid;
    RestoreCpuInter(t);
    return -ENOSYS;
}
int
sys_getpgrp (void)
{
    return -ENOSYS;
}
int
sys_setsid (void){
    return -ENOSYS;
}
int
sys_uname (struct utsname *name) {
    return -ENOSYS;
}
int
sys_umask (int mask){
    return -ENOSYS;
}