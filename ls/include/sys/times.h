#ifndef _SYS_TIMES_H
#define _SYS_TIMES_H

#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

struct tms {
	clock_t tms_utime;/*用戶程序cpu時間*/
	clock_t tms_stime;/*系统调用所耗费的cpu时间*/
	clock_t tms_cutime;/*已死掉子进程的cpu时间*/
	clock_t tms_cstime;/*已死掉子进程所耗费的系统调用 CPU 时间*/
};

clock_t times(struct tms *buf) __THROW;

__END_DECLS

#endif
