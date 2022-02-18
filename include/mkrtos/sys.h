

extern int sys_setup();        // 0 - 系统启动初始化设置函数。   (kernel/blk_drv/hd.c)
extern int sys_exit();         // 1 - 程序退出。                 (kernel/exit.c )
extern int sys_fork();         // 2 - 创建进程。                 (kernel/system_call.s)
extern int sys_read();         // 3 - 读文件。                   (fs/read_write.c)
extern int sys_write();        // 4 - 写文件。                   (fs/read_write.c)
extern int sys_open();         // 5 - 打开文件。                 (fs/open.c)
extern int sys_close();        // 6 - 关闭文件。                 (fs/open.c)
extern int sys_waitpid();      // 7 - 等待进程终止。             (kernel/exit.c)
extern int sys_creat();        // 8 - 创建文件。                 (fs/open.c)
extern int sys_link();         // 9 - 创建一个文件的硬连接。     (fs/namei.c)
extern int sys_unlink();       // 10 - 删除一个文件名(或删除文件)。 (fs/namei.c)
extern int sys_execve();       // 11 - 执行程序。                (kernel/system_call.s)
extern int sys_chdir();        // 12 - 更改当前目录。            (fs/open.c)
extern int sys_time();         // 13 - 取当前时间。              (kernel/sys.c)
extern int sys_mknod();        // 14 - 建立块/字符特殊文件。     (fs/namei.c)
extern int sys_chmod();        // 15 - 修改文件属性。            (fs/open.c)
extern int sys_chown();        // 16 - 修改文件宿主和所属组。    (fs/open.c)
extern int sys_break();        // 17 -                           (kernel/sys.c)*
extern int sys_stat();         // 18 - 使用路径名取文件状态信息。(fs/stat.c)
extern int sys_lseek();        // 19 - 重新定位读/写文件偏移。   (fs/read_write.c)
extern int sys_getpid();       // 20 - 取进程id。               (kernel/sched.c)
extern int sys_mount();        // 21 - 安装文件系统。            (fs/super.c)
extern int sys_umount();       // 22 - 卸载文件系统。            (fs/super.c)
extern int sys_setuid();       // 23 - 设置进程用户id。         (kernel/sys.c)
extern int sys_getuid();       // 24 - 取进程用户id。           (kernel/sched.c)
extern int sys_stime();        // 25 - 设置系统时间日期。        (kernel/sys.c)*
extern int sys_ptrace();       // 26 - 程序调试。                (kernel/sys.c)*
extern int sys_alarm();        // 27 - 设置报警。                (kernel/sched.c)
extern int sys_fstat();        // 28 - 使用文件句柄取文件的状态信息。(fs/stat.c)
extern int sys_pause();        // 29 - 暂停进程运行。            (kernel/sched.c)
extern int sys_utime();        // 30 - 改变文件的访问和修改时间。(fs/open.c)
extern int sys_stty();         // 31 - 修改终端行设置。          (kernel/sys.c)*
extern int sys_gtty();         // 32 - 取终端行设置信息。        (kernel/sys.c)*
extern int sys_access();       // 33 - 检查用户对一个文件的访问权限。(fs/open.c)
extern int sys_nice();         // 34 - 设置进程执行优先权。      (kernel/sched.c)
extern int sys_ftime();        // 35 - 取日期和时间。            (kernel/sys.c)*
extern int sys_sync();         // 36 - 同步高速缓冲与设备中数据。(fs/buffer.c)
extern int sys_kill();         // 37 - 终止一个进程。            (kernel/exit.c)
extern int sys_rename();       // 38 - 更改文件名。              (kernel/sys.c)*
extern int sys_mkdir();        // 39 - 创建目录。                (fs/namei.c)
extern int sys_rmdir();        // 40 - 删除目录。                (fs/namei.c)
extern int sys_dup();          // 41 - 复制文件句柄。            (fs/fcntl.c)
extern int sys_pipe();         // 42 - 创建管道。                (fs/pipe.c)
extern int sys_times();        // 43 - 取运行时间。              (kernel/sys.c)
extern int sys_prof();         // 44 - 程序执行时间区域。        (kernel/sys.c)*
extern int sys_brk();          // 45 - 修改数据段长度。          (kernel/sys.c)
extern int sys_setgid();       // 46 - 设置进程组id。            (kernel/sys.c)
extern int sys_getgid();       // 47 - 取进程组id。              (kernel/sched.c)
extern int sys_signal();       // 48 - 信号处理。                (kernel/signal.c)
extern int sys_geteuid();      // 49 - 取进程有效用户id。        (kenrl/sched.c)
extern int sys_getegid();      // 50 - 取进程有效组id。          (kenrl/sched.c)
extern int sys_acct();         // 51 - 进程记帐。                (kernel/sys.c)*
extern int sys_phys();         // 52 -                           (kernel/sys.c)*
extern int sys_lock();         // 53 -                           (kernel/sys.c)*
extern int sys_ioctl();        // 54 - 设备输入输出控制。        (fs/ioctl.c)
extern int sys_fcntl();        // 55 - 文件句柄控制操作。        (fs/fcntl.c)
extern int sys_mpx();          // 56 -                           (kernel/sys.c)*
extern int sys_setpgid();      // 57 - 设置进程组id。            (kernel/sys.c)
extern int sys_ulimit();       // 58 - 统计进程使用资源情况。     (kernel/sys.c)
extern int sys_uname();        // 59 - 显示系统信息。             (kernel/sys.c)
extern int sys_umask();        // 60 - 取默认文件创建属性码。     (kernel/sys.c)
extern int sys_chroot();       // 61 - 改变根目录。               (fs/open.c)
extern int sys_ustat();        // 62 - 取文件系统信息。           (fs/open.c)
extern int sys_dup2();         // 63 - 复制文件句柄。             (fs/fcntl.c)
extern int sys_getppid();      // 64 - 取父进程id。              (kernel/sched.c)
extern int sys_getpgrp();      // 65 - 取进程组id，等于getpgid(0)。(kernel/sys.c)
extern int sys_setsid();       // 66 - 在新会话中运行程序。       (kernel/sys.c)
extern int sys_sigaction();    // 67 - 改变信号处理过程。         (kernel/signal.c)
extern int sys_sgetmask();     // 68 - 取信号屏蔽码。             (kernel/signal.c)
extern int sys_ssetmask();     // 69 - 设置信号屏蔽码。           (kernel/signal.c)
extern int sys_setreuid();     // 70 - 设置真实与/或有效用户id。  (kernel/sys.c)
extern int sys_setregid();     // 71 - 设置真实与/或有效组id。    (kernel/sys.c)
extern int sys_sigpending();   // 73 - 检查暂未处理的信号。       (kernel/signal.c)
extern int sys_sigsuspend();   // 72 - 使用新屏蔽码挂起进程。     (kernel/signal.c)
extern int sys_sethostname();  // 74 - 设置主机名。               (kernel/sys.c)
extern int sys_setrlimit();    // 75 - 设置资源使用限制。         (kernel/sys.c)
extern int sys_getrlimit();    // 76 - 取得进程使用资源的限制。   (kernel/sys.c)
extern int sys_getrusage();    // 77 -
extern int sys_gettimeofday(); // 78 - 获取当日时间。             (kernel/sys.c)
extern int sys_settimeofday(); // 79 - 设置当日时间。             (kernel/sys.c)
extern int sys_getgroups();    // 80 - 取得进程所有组标识号。     (kernel/sys.c)
extern int sys_setgroups();    // 81 - 设置进程组标识号数组。     (kernel/sys.c)
extern int sys_select();       // 82 - 等待文件描述符状态改变。   (fs/select.c)
extern int sys_symlink();      // 83 - 建立符号链接。             (fs/namei.c，767)
extern int sys_lstat();        // 84 - 取符号链接文件状态。       (fs/stat.c，47)
extern int sys_readlink();     // 85 - 读取符号链接文件信息。     (fs/stat.c，69)
extern int sys_uselib();       // 86 - 选择共享库。
extern int sys_mmap();       // 90 - 选择共享库。
extern int sys_munmap();    //91
extern int sys_wait4();     //114-wait4

extern int sys_rt_sigaction(); //174
extern int sys_rt_sigreturn(void* psp);

extern int sys_readdir();
int sys_fchmod(unsigned int fd, mode_t mode);
extern int sys_fchdir();
int sys_statfs(const char * path, struct statfs * buf);
int sys_fstatfs(unsigned int fd, struct statfs * buf);
extern int sys_sigreturn();
typedef void* fn_ptr ;
// 系统调用函数指针表。用于系统调用中断处理程序(int 0x80)，作为跳转表。
fn_ptr sys_call_table[] = {sys_setup,//实现
                           sys_exit,//实现
                           sys_fork,//实现
                           sys_read,//实现
                           sys_write,//实现
                           sys_open,//实现
                           sys_close,//实现
                           sys_waitpid,//实现

                            sys_creat,//实现
                            sys_link,//实现
                           sys_unlink,//实现
                           NULL,//sys_execve,
                            sys_chdir,//实现
                            sys_time,//
                            sys_mknod,
                            sys_chmod,
                           NULL,

                           sys_break,
                           sys_stat,

                           sys_lseek,
                           sys_getpid,
                           sys_mount,
                           NULL,//sys_umount,
                           sys_setuid,
                           sys_getuid,
                           sys_stime,
                           sys_ptrace,
                           sys_alarm,
                           NULL,//sys_fstat,
                           NULL,//sys_pause,
                           sys_utime,
                           sys_stty,
                           sys_gtty,
                           sys_access,
                           sys_nice,
                           sys_ftime,
                           NULL,//sys_sync,
                           NULL, //sys_kill,
                           sys_rename,
                           sys_mkdir,
                           sys_rmdir,
                           sys_dup,
                           NULL,//sys_pipe,
                           sys_times,
                           sys_prof,
                           sys_brk,
                           sys_setgid,
                           sys_getgid,
                           sys_signal,
                           sys_geteuid,
                           sys_getegid,
                           sys_acct,
                           NULL,
//                           sys_phys,
                           sys_lock,
                           sys_ioctl,
                           sys_fcntl,
                           sys_mpx,
                           sys_setpgid,
                           sys_ulimit,
                           sys_uname,
                           sys_umask,
                           sys_chroot,
                           sys_ustat,
                           sys_dup2,
                           sys_getppid,
                           sys_getpgrp,
                           sys_setsid,
                           sys_sigaction,
                           sys_sgetmask,
                           sys_ssetmask,
                           sys_setreuid,
                           sys_setregid,
//                           sys_sigsuspend, sys_sigpending, sys_sethostname,
//                           sys_setrlimit, sys_getrlimit, sys_getrusage, sys_gettimeofday,
//                           sys_settimeofday, sys_getgroups, sys_setgroups, sys_select, sys_symlink,
//                           sys_lstat, sys_readlink, sys_uselib,

    [89]=sys_readdir,
    [90]=sys_mmap,
    [91]=sys_munmap,
    [94]=sys_fchmod,
    [99]=sys_statfs,
    [100]=sys_fstatfs,
    [114]=sys_wait4,
    [173]=sys_rt_sigreturn,
    [119]=sys_sigreturn,
    [133]=sys_fchdir,
    [174]=sys_rt_sigaction,
    [182]=sys_chown,

};
