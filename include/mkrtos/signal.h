#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <type.h>		// ����ͷ�ļ��������˻�����ϵͳ�������͡�

typedef int sig_atomic_t;	// �����ź�ԭ�Ӳ������͡�
typedef unsigned int sigset_t;	/* 32 bits */// �����źż����͡�

#define _NSIG 32		// �����ź����� -- 32 �֡�
#define NSIG _NSIG		// NSIG = _NSIG

// ������Щ��Linux 0.11 �ں��ж�����źš�
#define SIGHUP 1		// Hang Up -- �ҶϿ����ն˻���̡�
#define SIGINT 2		// Interrupt -- ���Լ��̵��жϡ�
#define SIGQUIT 3		// Quit -- ���Լ��̵��˳���
#define SIGILL 4		// Illeagle -- �Ƿ�ָ�
#define SIGTRAP 5		// Trap -- ���ٶϵ㡣
#define SIGABRT 6		// Abort -- �쳣������
#define SIGIOT 6		// IO Trap -- ͬ�ϡ�
#define SIGUNUSED 7		// Unused -- û��ʹ�á�
#define SIGFPE 8		// FPE -- Э����������
#define SIGKILL 9		// Kill -- ǿ�Ƚ�����ֹ��
#define SIGUSR1 10		// User1 -- �û��ź�1�����̿�ʹ�á�
#define SIGSEGV 11		// Segment Violation -- ��Ч�ڴ����á�
#define SIGUSR2 12		// User2 -- �û��ź�2�����̿�ʹ�á�
#define SIGPIPE 13		// Pipe -- �ܵ�д�����޶��ߡ�
#define SIGALRM 14		// Alarm -- ʵʱ��ʱ��������
#define SIGTERM 15		// Terminate -- ������ֹ��
#define SIGSTKFLT 16	// Stack Fault -- ջ����Э����������
#define SIGCHLD 17		// Child -- �ӽ���ֹͣ����ֹ��
#define SIGCONT 18		// Continue -- �ָ����̼���ִ�С�
#define SIGSTOP 19		// Stop -- ֹͣ���̵�ִ�С�
#define SIGTSTP 20		// TTY Stop -- tty ����ֹͣ���̣��ɺ��ԡ�
#define SIGTTIN 21		// TTY In -- ��̨�����������롣
#define SIGTTOU 22		// TTY Out -- ��̨�������������

/* Ok, I haven't implemented sigactions, but trying to keep headers POSIX */
/* OK���һ�û��ʵ��sigactions �ı��ƣ�����ͷ�ļ�����ϣ������POSIX ��׼ */
#define SA_NOCLDSTOP 1		// ���ӽ��̴���ֹͣ״̬���Ͳ���SIGCHLD ����
#define SA_NOMASK 0x40000000	// ����ֹ��ָ�����źŴ������(�źž��)�����յ����źţ�û��ʵ��
#define SA_ONESHOT 0x80000000	// �źž��һ�������ù��ͻָ���Ĭ�ϴ�������

// ���²�������sigprocmask()-- �ı������źż�(������)����Щ�������Ըı�ú�������Ϊ��
#define SIG_BLOCK 0		/* for blocking signals */
// �������źż��м��ϸ������źż���
#define SIG_UNBLOCK 1		/* for unblocking signals */
// �������źż���ɾ��ָ�����źż���
#define SIG_SETMASK 2		/* for setting the signal mask */
// ���������źż����ź������룩��

#define SIG_DFL ((void (*)(int))0)	/* default signal handling */
// Ĭ�ϵ��źŴ�������źž������
#define SIG_IGN ((void (*)(int))1)	/* ignore signal */
// �����źŵĴ������

// ������sigaction �����ݽṹ��
// sa_handler �Ƕ�Ӧĳ�ź�ָ��Ҫ��ȡ���ж��������������SIG_DFL��������SIG_IGN ������
// ���źţ�Ҳ������ָ������źź�����һ��ָ�롣
// sa_mask �����˶��źŵ������룬���źų���ִ��ʱ����������Щ�źŵĴ���
// sa_flags ָ���ı��źŴ�����̵��źż���������37-39 �е�λ��־����ġ�
// sa_restorer �ָ�����ָ�룬�����ڱ���ԭ���صĹ���ָ�롣
// ���⣬���𴥷��źŴ�����ź�Ҳ��������������ʹ����SA_NOMASK ��־��
struct sigaction
{
    void (*sa_handler) (int);
    int sa_flags;
    void (*sa_restorer) (void);
    sigset_t sa_mask;
};

void sig_chld(struct task *tk);


#endif /* _SIGNAL_H */
