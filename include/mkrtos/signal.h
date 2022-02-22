
#include <signal.h>
//#ifndef _SIGNAL_H
//#define _SIGNAL_H
//
//#include <type.h>		// 类型头文件。定义了基本的系统数据类型。
//
//typedef int sig_atomic_t;	// 定义信号原子操作类型。
//typedef unsigned int sigset_t;	/* 32 bits */// 定义信号集类型。
//
//#define _NSIG 32		// 定义信号种类 -- 32 种。
//#define NSIG _NSIG		// NSIG = _NSIG
//
//// 以下这些是Linux 0.11 内核中定义的信号。
//#define SIGHUP 1		// Hang Up -- 挂断控制终端或进程。
//#define SIGINT 2		// Interrupt -- 来自键盘的中断。
//#define SIGQUIT 3		// Quit -- 来自键盘的退出。
//#define SIGILL 4		// Illeagle -- 非法指令。
//#define SIGTRAP 5		// Trap -- 跟踪断点。
//#define SIGABRT 6		// Abort -- 异常结束。
//#define SIGIOT 6		// IO Trap -- 同上。
//#define SIGUNUSED 7		// Unused -- 没有使用。
//#define SIGFPE 8		// FPE -- 协处理器出错。
//#define SIGKILL 9		// Kill -- 强迫进程终止。
//#define SIGUSR1 10		// User1 -- 用户信号1，进程可使用。
//#define SIGSEGV 11		// Segment Violation -- 无效内存引用。
//#define SIGUSR2 12		// User2 -- 用户信号2，进程可使用。
//#define SIGPIPE 13		// Pipe -- 管道写出错，无读者。
//#define SIGALRM 14		// Alarm -- 实时定时器报警。
//#define SIGTERM 15		// Terminate -- 进程终止。
//#define SIGSTKFLT 16	// Stack Fault -- 栈出错（协处理器）。
//#define SIGCHLD 17		// Child -- 子进程停止或被终止。
//#define SIGCONT 18		// Continue -- 恢复进程继续执行。
//#define SIGSTOP 19		// Stop -- 停止进程的执行。
//#define SIGTSTP 20		// TTY Stop -- tty 发出停止进程，可忽略。
//#define SIGTTIN 21		// TTY In -- 后台进程请求输入。
//#define SIGTTOU 22		// TTY Out -- 后台进程请求输出。
//#define SIGURG		23
//#define SIGXCPU		24
//#define SIGXFSZ		25
//#define SIGVTALRM	26
//#define SIGPROF		27
//#define SIGWINCH	28
//#define SIGIO		29
//#define SIGPWR		30
//#define SIGSYS		31
////当儿童停止或停止的儿童继续时，不要产生SIGCHLD。
//#define SA_NOCLDSTOP	0x00000001
////如果清零并捕捉到信号，则进入信号捕捉函数为：void func(int signo);
////其中signo是信号捕获函数的唯一参数。
////在这种情况下，必须使用sa_handler成员来描述信号捕获功能，并且应用程序不得修改sa_sigaction 成员。
////如果设置了 SA_SIGINFO 并捕获了信号，则信号捕获函数将输入为：void func(int signo, siginfo_t *info, void *context);
////其中两个额外的参数被传递给信号捕获函数
////。第二个参数将指向 siginfo_t类型的对象， 解释信号生成的原因；第三个参数可以转换为指向 ucontext_t类型对象的指针， 以引用在传递信号时中断的接收进程的上下文。
////在这种情况下，必须使用sa_sigaction成员来描述信号捕获功能，并且应用程序不得修改 sa_handler成员。
////si_signo成员包含系统生成的信号编号。
////si_errno成员可能包含依赖于实现的附加错误信息；如果非零，则它包含一个错误号，用于标识导致生成信号的条件。
////si_code成员包含标识信号原因的代码。
////如果si_code的值小于等于 0，那么信号是由进程产生的，si_pid和si_uid分别表示进程 ID 和发送者的真实用户 ID。
////<signal.h> 标头描述包含有关 siginfo_t 类型 元素的信号特定内容的 信息。
//#define SA_SIGINFO	0x00000004
////如果设置了，并且用sigaltstack ( )声明了一个备用的信号栈，
////那么该信号将被传递给该栈上的调用进程。否则。信号将在当前堆栈中传递。
////如果sig是SIGCHLD，并且SA_flags中没有设置SA_NOCLDSTOP标志，
////并且实现支持SIGCHLD信号，那么只要它的任何子进程停止，
////就应该为调用进程产生一个SIGCHLD信号，并且只要它的任何停止的子进程继续，
////就可能为调用进程产生一个SIGCHLD信号。如果sig是SIGCHLD，
////并且SA_flags中设置了SA_NOCLDSTOP标志，那么实现不应该以这种方式产生SIGCHLD信号。
//#define SA_ONSTACK      0x08000000
////如果设置，信号的处置应被重置为SIG_DFL，并且在进入信号处理程序时应清除SA_SIGINFO标志。
////note:SIGILL和SIGTRAP在交付时不能自动复位。系统会默默地执行这一限制。
////此外，如果设置了此标志， sigaction() 的行为就好像也设置了 SA_NODEFER 标志一样。
//#define SA_RESETHAND    0x80000000
//// 不阻止在指定的信号处理程序(信号句柄)中再收到该信号
//#define SA_NODEFER      0x40000000
//
//#define SA_NOMASK       SA_NODEFER
//#define SA_ONESHOT SA_RESETHAND	// 信号句柄一旦被调用过就恢复到默认处理句柄。
//
//// 以下参数用于sigprocmask()-- 改变阻塞信号集(屏蔽码)。这些参数可以改变该函数的行为。
//#define SIG_BLOCK 0		/* for blocking signals */
//// 在阻塞信号集中加上给定的信号集。
//#define SIG_UNBLOCK 1		/* for unblocking signals */
//// 从阻塞信号集中删除指定的信号集。
//#define SIG_SETMASK 2		/* for setting the signal mask */
//// 设置阻塞信号集（信号屏蔽码）。
//
//#define SIG_DFL ((void (*)(int))0)	/* default signal handling */
//// 默认的信号处理程序（信号句柄）。
//#define SIG_IGN ((void (*)(int))1)	/* ignore signal */
//// 忽略信号的处理程序。
//
//// 下面是sigaction 的数据结构。
//// sa_handler 是对应某信号指定要采取的行动。可以是上面的SIG_DFL，或者是SIG_IGN 来忽略
//// 该信号，也可以是指向处理该信号函数的一个指针。
//// sa_mask 给出了对信号的屏蔽码，在信号程序执行时将阻塞对这些信号的处理。
//// sa_flags 指定改变信号处理过程的信号集。它是由37-39 行的位标志定义的。
//// sa_restorer 恢复过程指针，是用于保存原返回的过程指针。
//// 另外，引起触发信号处理的信号也将被阻塞，除非使用了SA_NOMASK 标志。
//struct sigaction
//{
//    union{
//        //指向信号捕捉函数或宏SIG_IGN或SIG_DFL之一的指针。
//        void (*sa_handler) (int);
//        //指向信号捕获函数的指针。
//        void (*sa_sigaction) (void);
//    };
//    //影响信号行为的特殊标志。
//    int sa_flags;
//    //在执行信号捕捉的过程中要阻断的额外信号集
//    sigset_t sa_mask;
//    void (*sa_restorer)(void);
//};
//
//
//
//void sig_chld(struct task *tk);
//
//
//#endif /* _SIGNAL_H */
