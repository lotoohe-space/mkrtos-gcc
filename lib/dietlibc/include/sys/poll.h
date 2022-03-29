#ifndef _SYS_POLL_H
#define _SYS_POLL_H

#include <sys/cdefs.h>

__BEGIN_DECLS

enum {
  POLLIN	= 0x0001,
#define POLLIN		POLLIN /*有数据可读*/
  POLLPRI	= 0x0002,
#define POLLPRI		POLLPRI /*有紧急数据可读*/
  POLLOUT	= 0x0004,
#define POLLOUT		POLLOUT /*写数据不会导致阻塞，意思就是可以写数据了*/
  POLLERR	= 0x0008,
#define POLLERR		POLLERR/*指定的文件描述符发生错误*/
  POLLHUP	= 0x0010,
#define POLLHUP		POLLHUP/*指定的文件描述符挂起事件*/
  POLLNVAL	= 0x0020,
#define POLLNVAL	POLLNVAL/*无效的请求，打不开指定的文件描述符*/
  POLLRDNORM	= 0x0040,
#define POLLRDNORM	POLLRDNORM/*有普通数据可读，等效与POLLIN*/
  POLLRDBAND	= 0x0080,
#define POLLRDBAND	POLLRDBAND
  POLLWRBAND	= 0x0200,
#define POLLWRBAND	POLLWRBAND
  POLLMSG	= 0x0400,
#define POLLMSG		POLLMSG
/* POLLREMOVE is for /dev/epoll (/dev/misc/eventpoll),
 * a new event notification mechanism for 2.6 */
  POLLREMOVE	= 0x1000,
#define POLLREMOVE	POLLREMOVE
};

#if defined(__sparc__) || defined (__mips__)
#define POLLWRNORM	POLLOUT
#else
#define POLLWRNORM	0x0100
#endif

struct pollfd {
  int fd;
  short events;
  short revents;
};

extern int poll(struct pollfd *ufds, unsigned int nfds, int timeout) __THROW;

__END_DECLS

#endif	/* _SYS_POLL_H */
