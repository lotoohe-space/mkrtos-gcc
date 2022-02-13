//
// Created by Administrator on 2022/1/9.
//

#include <errno.h>
#include <mkrtos/task.h>
#include <arch/arch.h>
// �������ں�ʱ�䡣
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

// ���ڵ�ǰ���̶��ӽ��̽��е���(degugging)��
int
sys_ptrace ()
{
    return -ENOSYS;
}

// �ı䲢��ӡ�ն������á�
int
sys_stty ()
{
    return -ENOSYS;
}

// ȡ�ն���������Ϣ��
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
// ���ý������(gid)���������û�г����û���Ȩ��������ʹ��setgid()������Чgid
// ��effective gid������Ϊ���䱣��gid(saved gid)����ʵ��gid(real gid)�����������
// �����û���Ȩ����ʵ��gid����Чgid �ͱ���gid �������óɲ���ָ����gid��
int
sys_setgid (int gid)
{
    return (sys_setregid (gid, gid));
}

// �򿪻�رս��̼��ʹ��ܡ�
int
sys_acct ()
{
    return -ENOSYS;
}

// ӳ�����������ڴ浽���̵������ַ�ռ䡣
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
// ���ش�1970 ��1 ��1 ��00:00:00 GMT ��ʼ��ʱ��ʱ��ֵ���룩�����tloc ��Ϊnull����ʱ��ֵ
// Ҳ�洢�����
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
// ��ȡ��ǰ����ʱ�䡣tms �ṹ�а����û�ʱ�䡢ϵͳʱ�䡢�ӽ����û�ʱ�䡢�ӽ���ϵͳʱ�䡣
int
sys_times (struct tms *tbuf)
{
    return -ENOSYS;
}
// ������end_data_seg ��ֵ��������ϵͳȷʵ���㹻���ڴ棬���ҽ���û�г�Խ��������ݶδ�С
// ʱ���ú����������ݶ�ĩβΪend_data_seg ָ����ֵ����ֵ������ڴ����β����ҪС�ڶ�ջ
// ��β16KB������ֵ�����ݶε��½�βֵ���������ֵ��Ҫ��ֵ��ͬ��������д�������
// �ú����������û�ֱ�ӵ��ã�����libc �⺯�����а�װ�����ҷ���ֵҲ��һ����
int
sys_brk (unsigned long end_data_seg)
{
    return -ENOSYS;
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