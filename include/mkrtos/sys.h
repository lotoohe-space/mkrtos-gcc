

extern int sys_setup();        // 0 - 系统启动初始化设置函数。
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
extern int sys_truncate();//92
extern int sys_ftruncate();//93
extern int sys_fchown();//95
extern int sys_getpriority(); //96
extern int sys_setpriority(); //97
extern int sys_socketcall();
extern int sys_wait4();     //114-wait4
extern int sys_ipc();       //117
extern int sys_fsync();
extern int sys_getdents();
extern int sys_clone(); //120
extern int sys_sigprocmask();
extern int sys_sched_yield();//158
extern int sys_nanosleep(); //162
extern int sys_mremap();//163
extern int sys_poll();//168
extern int sys_rt_sigaction(); //174
extern int sys_rt_sigprocmask();
extern int sys_rt_sigsuspend();//179
extern int sys_getcwd();//183
extern int sys_getenv();

extern int sys_rt_sigreturn(void* psp);

extern int sys_readdir();
int sys_fchmod(unsigned int fd, mode_t mode);
extern int sys_fchdir();
int sys_statfs(const char * path, struct statfs * buf);
int sys_fstatfs(unsigned int fd, struct statfs * buf);
extern int sys_sigreturn();

extern int sys_socket();//281
extern int sys_bind();//282
extern int sys_connect();//283
extern int sys_listen();//284
extern int sys_accept();//285
extern int sys_getsockname();//286
extern int sys_getpeername();//287
extern int sys_socketpair();//288
extern int sys_send();//289
extern int sys_sendto();//290
extern int sys_recv();//291
extern int sys_recvfrom();//292
extern int sys_shutdown();//293
extern int sys_setsockopt();//294
extern int sys_getsockopt();//295
extern int sys_sendmsg();//296
extern int sys_recvmsg();//297


//extern int sys_net_init();

typedef void* fn_ptr ;

const fn_ptr sys_call_table[] = {
        [0]=sys_setup,//实现
        [1]=sys_exit,//实现
        [2]=sys_fork,//实现
        [3]=sys_read,//实现
        [4]=sys_write,//实现
        [5]=sys_open,//实现
        [6]=sys_close,//实现
        [7]=sys_waitpid,//实现
        [8]= sys_creat,//实现
        [9]=sys_link,//实现
        [10]=sys_unlink,//实现
        [11]=sys_execve,
        [12]=sys_chdir,//实现
        [13]=sys_time,//
        [14]=sys_mknod,
        [15]=sys_chmod,
        [16]= NULL,//sys_chown
        [17]=sys_break,
        [18]=sys_stat,
        [19]=sys_lseek,
        [20]=sys_getpid,
        [21]=sys_mount,
        [22]=NULL,//sys_umount,
        [23]=sys_setuid,
        [24]=sys_getuid,
        [25]=sys_stime,
        [26]=sys_ptrace,
        [27]=sys_alarm,
        [28]=sys_fstat,
        [29]=sys_pause,
        [30]=sys_utime,
        [31]=sys_stty,
        [32]=sys_gtty,
        [33]=sys_access,
        [34]=sys_nice,
        [35]=sys_ftime,
        [36]=sys_sync,
        [37]=sys_kill,
        [38]=sys_rename,
        [39]=sys_mkdir,
        [40]=sys_rmdir,
        [41]=sys_dup,
        [42]=sys_pipe,
        [43]=sys_times,
        [44]=sys_prof,
        [45]=sys_brk,
        [46]=sys_setgid,
        [47]=sys_getgid,
        [48]=sys_signal,
        [49]=sys_geteuid,
        [50]=sys_getegid,
        [51]=sys_acct,
        [52]NULL,//sys_phys,
        [53]=sys_lock,
        [54]=sys_ioctl,
        [55]=sys_fcntl,
        [56]=sys_mpx,
        [57]=sys_setpgid,
        [58]=sys_ulimit,
        [59]=sys_uname,
        [60]=sys_umask,
        [61]=sys_chroot,
        [62]=sys_ustat,
        [63]=sys_dup2,
        [64]=sys_getppid,
        [65]=sys_getpgrp,
        [66]=sys_setsid,
        [67]=sys_sigaction,
        [68]=sys_sgetmask,
        [69]=sys_ssetmask,
        [70]=sys_setreuid,
        [71]=sys_setregid,
        [72]=sys_sigsuspend,
//                           sys_sigpending,
//                           sys_sethostname,
//                           sys_setrlimit,
//                           sys_getrlimit,
//                           sys_getrusage,
       [78]=sys_gettimeofday,
       [79]=sys_settimeofday,
//                           sys_getgroups,
//                           sys_setgroups,
        [82]=sys_select,
        [83]=sys_symlink,
        [84]=sys_lstat,
//                           sys_readlink,
//                           sys_uselib,
        [89]=sys_readdir,
        [90]=sys_mmap,
        [91]=sys_munmap,
        [92]=sys_truncate,
        [93]=sys_ftruncate,
        [94]=sys_fchmod,
        [95]=sys_fchown,
        [96]=sys_getpriority,
        [97]=sys_setpriority,
        [99]=sys_statfs,
        [100]=sys_fstatfs,
        [102]=sys_socketcall,
        [106]=sys_stat,
        [107]=sys_lstat,
        [114]=sys_wait4,
        [117]=sys_ipc,
        [118]=sys_fsync,
        [119]=sys_sigreturn,
        [120]=sys_clone,
        [122]=sys_uname,
        [126]=sys_sigprocmask,
        [133]=sys_fchdir,
        [141]=sys_getdents,
        [158]=sys_sched_yield,
        [162]=sys_nanosleep,
        [163]=sys_mremap,
        [168]=sys_poll,
        [173]=sys_rt_sigreturn,
        [174]=sys_rt_sigaction,
        [175]=sys_rt_sigprocmask,
        [179]=sys_rt_sigsuspend,
        [182]=sys_chown,
        [183]=sys_getcwd,
        [281]=sys_socket,
        [282]=sys_bind,
        [283]=sys_connect,
        [284]=sys_listen,
        [285]=sys_accept,
        [286]=sys_getsockname,
        [287]=sys_getpeername,
        [288]=sys_socketpair,
        [289]=sys_send,
        [290]=sys_sendto,
        [291]=sys_recv,
        [292]=sys_recvfrom,
        [293]=sys_shutdown,
        [294]=sys_setsockopt,
        [295]=sys_getsockopt,
        [296]=sys_sendmsg,
        [297]=sys_recvmsg,
};
