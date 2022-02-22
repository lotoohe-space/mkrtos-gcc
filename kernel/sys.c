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

// ������
int sys_ftime ()
{
    return -ENOSYS;
}

//������
int sys_break ()
{
    return -ENOSYS;
}

// ���ڵ�ǰ���̶��ӽ��̽��е���(degugging)��
int sys_ptrace (int request,int pid,int addr,int data)
{
    return -ENOSYS;
}

// �ı䲢��ӡ�ն������á�
int sys_stty ()
{
    return -ENOSYS;
}

// ȡ�ն���������Ϣ��
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
// ���ý������(gid)���������û�г����û���Ȩ��������ʹ��setgid()������Чgid
// ��effective gid������Ϊ���䱣��gid(saved gid)����ʵ��gid(real gid)�����������
// �����û���Ȩ����ʵ��gid����Чgid �ͱ���gid �������óɲ���ָ����gid��
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

// �򿪻�رս��̼��ʹ��ܡ�
int sys_acct ()
{
    return -ENOSYS;
}

// ӳ�����������ڴ浽���̵������ַ�ռ䡣
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
// ���ش�1970 ��1 ��1 ��00:00:00 GMT ��ʼ��ʱ��ʱ��ֵ���룩�����tloc ��Ϊnull����ʱ��ֵ
// Ҳ�洢�����
int sys_time (time_t *tloc)
{
    time_t tick;
    //rtc.c
    extern uint32_t rtc_get_tick(void);
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
//�����û�id
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
    if(!CUR_TASK->is_s_user){
        return -EPERM;
    }
    //rtc.c
    extern uint32_t rtc_set_tick(uint32_t tick);
    rtc_set_tick(*tptr);
    return -ENOSYS;
}
// ��ȡ��ǰ����ʱ�䡣tms �ṹ�а����û�ʱ�䡢ϵͳʱ�䡢�ӽ����û�ʱ�䡢�ӽ���ϵͳʱ�䡣
int sys_times (struct tms *tbuf)
{
    if(!tbuf){
        return -EINVAL;
    }
    tbuf->tms_utime=CUR_TASK->runCount;
    //��������������˵�ɣ����ں���ûɶ��
    tbuf->tms_cstime=0;
    tbuf->tms_cutime=0;
    tbuf->tms_stime=0;
    return -ENOSYS;
}
// ������end_data_seg ��ֵ��������ϵͳȷʵ���㹻���ڴ棬���ҽ���û�г�Խ��������ݶδ�С
// ʱ���ú����������ݶ�ĩβΪend_data_seg ָ����ֵ����ֵ������ڴ����β����ҪС�ڶ�ջ
// ��β16KB������ֵ�����ݶε��½�βֵ���������ֵ��Ҫ��ֵ��ͬ��������д�������
// �ú����������û�ֱ�ӵ��ã�����libc �⺯�����а�װ�����ҷ���ֵҲ��һ����
int sys_brk (unsigned long end_data_seg)
{
    //û�������ڴ�ĸ��ֱ�ӷ��سɹ�
    return 0;
}
/**
 * ������ID
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

//û��÷ŵ��Ķ����ȷŵ������
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
 * �ڽ��̴���һ�����ļ�����Ŀ¼ʱ����һ����ʹ���ļ���ʽ���������� (����3 . 3��3 . 4�ڣ�
 * ����������˵���� o p e n��c r e a t��������������������һ������ m o d e����ָ�������ļ��Ĵ�ȡ
 * ���Ȩλ)�����ǽ���4 . 2 0��˵����δ���һ����Ŀ¼�����ļ���ʽ������������Ϊ 1��λ����
 * �ļ�m o d e�е���Ӧλ��һ����ת��0��
    */
mode_t sys_umask (mode_t mask){
    mode_t old_mask;
    old_mask=CUR_TASK->euid;
    CUR_TASK->euid=mask;
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
    if(CUR_TASK->parentTask){
        return CUR_TASK->parentTask->PID;
    }else{
        return -1;
    }
}

// ȡ�û���uid��
int32_t
sys_getuid (void)
{
    return CUR_TASK->ruid;
}

// ȡeuid��
int32_t
sys_geteuid (void)
{
    return CUR_TASK->euid;
}

// ȡ���gid��
int32_t
sys_getgid (void)
{
    return CUR_TASK->rgid;
}

// ȡegid��
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

//ִ��һ���µĳ���
int sys_execve(const char *filename, char *const argv[ ], char *const envp[ ]){

    return -ENOSYS;
}
