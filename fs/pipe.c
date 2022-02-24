//
// Created by zz on 2022/2/23.
//
#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <fcntl.h>
#include <mkrtos/mem.h>

struct pipe_struct{
    uint8_t *data;
    uint32_t data_len;
    uint32_t rear;
    uint32_t front;
    uint32_t lock;

    int fd[2];//读写的fd
    //pipe中的next
    struct task* pipe_r_next;
    //pipe中的next
    struct task* pipe_w_next;

    struct wait_queue* r_wait;
    struct wait_queue* w_wait;
};

#define PIPE_LEN(a) ((a->rear-a->front+a->data_len)%a->data_len)
#define MIN(a,b) ((a)<(b)?(a):(b))
#define FIFO_SIZE 512

extern struct inode_operations pipe_iops;
static int pipe_open (struct inode * inode, struct file * fp);
//pipe如何知道没有进程来读或者来写了？
//只有在fork的时候将当前进程添加到pipe的链表中去。

/**
 * 对pipe进行fork
 * @param inode
 */
void do_fork_pipe(struct inode *inode){
    struct pipe_struct *pipe;
    if(!inode){
        return ;
    }
    pipe=inode->i_fs_priv_info;

    if(!S_ISFIFO(inode->i_type_mode)){
        return ;
    }
    if(atomic_test(&inode->i_used_count,0)){
        return ;
    }

    if(!pipe->pipe_r_next){
        CUR_TASK->pipe_r_next=NULL;
        pipe->pipe_r_next=CUR_TASK;
    }else{
        CUR_TASK->pipe_r_next=pipe->pipe_r_next;
        pipe->pipe_r_next=CUR_TASK;
    }

    if(!pipe->pipe_w_next){
        CUR_TASK->pipe_w_next=NULL;
        pipe->pipe_w_next=CUR_TASK;
    }else{
        CUR_TASK->pipe_w_next=pipe->pipe_w_next;
        pipe->pipe_w_next=CUR_TASK;
    }

    return ;
}

static void remove_task(struct task** tasks,struct inode* inode){
    struct task *temp;
    struct task *prev=NULL;
//    struct pipe_struct *pipe;
    if(!tasks){
        return ;
    }
//    pipe=inode->i_fs_priv_info;

    temp=*tasks;
    while(temp){
        if(temp==CUR_TASK) {
            if (prev==NULL) {
                //删除的第一个
                (*tasks)->pipe_r_next=temp->pipe_r_next;
                break;
            }else{
                prev->pipe_r_next=temp->pipe_r_next;
                break;
            }
        }
        prev=temp;
        temp=temp->pipe_r_next;
    }
}

/**
 * 获取一个管道的inode
 * @return
 */
struct inode * get_pipe_inode(void){
    struct inode *r_inode;
    r_inode=get_empty_inode();
    //初始化pipe_inode信息
    //使用计数
    atomic_set(&(r_inode->i_used_count),1);
    atomic_set(&(r_inode->i_lock),0);
    r_inode->i_file_size=0;
    r_inode->i_wait_q = NULL;
    r_inode->i_type_mode = MK_MODE(S_IFIFO,0777);
    //填充参数
    r_inode->i_sb = NULL;
    //这里设置一个无效的inode号码，这样不会与文件系统的iode号码冲突
    r_inode->i_no = -1;
    r_inode->i_hlink = 0;
    r_inode->i_fs_priv_info = NULL;
    r_inode->i_ops = &pipe_iops;

    return r_inode;
}
/**
 * 创建pipe管道
 * @param fd
 * @return
 */
int32_t sys_pipe(int32_t *fd){
    int32_t fds[2];
    int32_t i;
    struct inode *new_inode;
    if(!fd){
        return -EINVAL;
    }
    //获取两个fd
    for( i=0;i<NR_FILE;i++){
        struct file *f=&(CUR_TASK->files[i]);
        if(!f->used){
            fds[0]=i;
            f->used=1;
        }
    }
    if(i==NR_FILE){
        return -EMFILE;
    }
    for(i=0;i<NR_FILE;i++){
        struct file *f=&(CUR_TASK->files[i]);
        if(!f->used){
            fds[1]=i;
            f->used=1;
        }
    }
    if(i==NR_FILE){
        CUR_TASK->files[fds[0]].used=0;
        return -EMFILE;
    }
    new_inode=get_pipe_inode();

    //初始化信息
    CUR_TASK->files[fds[0]].f_inode=new_inode;
    CUR_TASK->files[fds[0]].f_flags=O_RDWR;
    CUR_TASK->files[fds[0]].f_mode=0777;
    CUR_TASK->files[fds[0]].f_ofs=0;
    CUR_TASK->files[fds[0]].f_op=new_inode->i_ops->default_file_ops;
    CUR_TASK->files[fds[0]].f_rdev=0;

    CUR_TASK->files[fds[1]].f_inode=new_inode;
    CUR_TASK->files[fds[1]].f_flags=O_RDWR;
    CUR_TASK->files[fds[1]].f_mode=0777;
    CUR_TASK->files[fds[1]].f_ofs=0;
    CUR_TASK->files[fds[1]].f_op=new_inode->i_ops->default_file_ops;
    CUR_TASK->files[fds[1]].f_rdev=0;

    //打开pipe
    if(pipe_open(new_inode,&CUR_TASK->files[fds[0]])<0){
        CUR_TASK->files[fds[0]].used=0;
        CUR_TASK->files[fds[1]].used=0;
        puti(new_inode);
        return -ENOMEM;
    }
    atomic_inc(&new_inode->i_used_count);
    struct pipe_struct *pipe;
    pipe=new_inode->i_fs_priv_info;
    //保存两个fd
    pipe->fd[0]=fds[0];
    pipe->fd[1]=fds[1];

    fd[0]=fds[0];
    fd[1]=fds[1];

    return 0;
}



/**
 * pipe读数据，如果所有的
 * @param inode
 * @param fp
 * @param buf
 * @param cn
 * @return
 */
static int pipe_read (struct inode *inode, struct file *fp, char * buf, int cn){
    struct pipe_struct *pipe;
    int read_inx=0;
    int read_cn=cn;
    if(!cn){
        return 0;
    }
    pipe=inode->i_fs_priv_info;
    again:
    if(pipe->rear==pipe->front){
        if(!pipe->pipe_w_next){
            //没有写进程则直接返回0
            return 0;
        }
        //没有数据则等待
        struct wait_queue* wait={CUR_TASK,NULL};
//        pipe->r_wait_tk=CUR_TASK;
        add_wait_queue(&pipe->r_wait,wait);
        task_suspend();
        task_sche();
        remove_wait_queue(&pipe->r_wait,wait);
        goto again;
    }
    again_lock:
    //如果所
    if(atomic_test_set(&pipe->lock,1)){
        int i;
        int read_len;
        read_len=MIN(read_cn,PIPE_LEN(pipe));
        for(i=read_inx;i<read_len+read_inx;i++,read_inx++){
            buf[i]= pipe->data[pipe->front];
            pipe->front=(pipe->front+1)%pipe->data_len;
        }
        atomic_set(&pipe->lock,0);
        wake_up(pipe->w_wait);
//        task_run_1(pipe->w_wait_tk);
        if(read_inx>=cn){
            return read_inx;
        }else{
            read_cn=cn-read_inx;
            if(!pipe->pipe_w_next){
                //没有写进程则直接返回
                return read_inx;
            }
            //没有读够，继续等待
            goto again;
        }
    }else{
        struct wait_queue* wait={CUR_TASK,NULL};
//        pipe->r_wait_tk=CUR_TASK;
        add_wait_queue(&pipe->r_wait,wait);
        task_suspend();
        task_sche();
        remove_wait_queue(&pipe->r_wait,wait);
        goto again_lock;
    }
    atomic_set(&pipe->lock,0);
    return 0;
}
/**
 * pipe写函数
 * @param inode
 * @param fp
 * @param buf
 * @param cn
 * @return
 */
static int pipe_write (struct inode *inode, struct file *fp, char *buf, int cn){
    struct pipe_struct *pipe;
    int write_cn=cn;
    int write_inx=0;
    pipe=inode->i_fs_priv_info;
    if(!pipe->pipe_w_next){
        //发送信号SIGPIPE
        inner_set_sig(SIGPIPE);
        //并返回EPIPE
        return -EPIPE;
    }
    again:
    if((pipe->rear+1)%pipe->data_len==pipe->front) {
        //满了，等待
        struct wait_queue* wait={CUR_TASK,NULL};
        add_wait_queue(&pipe->w_wait,wait);
        task_suspend();
        task_sche();
        remove_wait_queue(&pipe->w_wait,wait);
        goto again;
    }
    again_lock:
    //如果所
    if(atomic_test_set(&pipe->lock,1)){
        int i;
        int write_len;
        write_len=MIN(write_cn,pipe->data_len-PIPE_LEN(pipe));
        for(i=write_inx;i<write_len+write_inx;i++,write_inx++){
            pipe->data[pipe->rear]=buf[i];
            pipe->rear=(pipe->rear+1)%pipe->data_len;
        }
        atomic_set(&pipe->lock,0);
        wake_up(pipe->r_wait);
//        task_run_1(pipe->r_wait_tk);
        if(write_inx>=cn){
            return write_inx;
        }else{
            write_cn=cn-write_inx;
            goto again;
        }
    }else{
        struct wait_queue* wait={CUR_TASK,NULL};
        add_wait_queue(&pipe->w_wait,wait);
        task_suspend();
        task_sche();
        remove_wait_queue(&pipe->w_wait,wait);
        goto again_lock;
    }
    atomic_set(&pipe->lock,0);
    return 0;
}
/**
 * pipe 打开函数
 * @param inode
 * @param fp
 * @return
 */
static int pipe_open(struct inode * inode, struct file * fp){
    struct pipe_struct *pipe;

    pipe= OSMalloc(sizeof(struct pipe_struct));
    if(!pipe){
        return -ENOMEM;
    }
    memset(pipe,0,sizeof(struct pipe_struct));
    pipe->data= OSMalloc(FIFO_SIZE);
    if(!pipe->data){
        OSFree(pipe);
        return -ENOMEM;
    }
    pipe->data_len=FIFO_SIZE;
    inode->i_fs_priv_info=pipe;

    return 0;
}
/**
 * 释放资源
 * @param inode
 * @param fp
 */
static void pipe_release (struct inode * inode, struct file * fp){
    int i;
    struct pipe_struct* pipe;

    //查找当前fp的inx
    for(i=0;i<NR_FILE;i++){
        if(CUR_TASK->files[i].used&& (&(CUR_TASK->files[i])==fp)){
            break;
        }
    }
    if(i==NR_FILE){
        return ;
    }
    //判断是读端还是写端，并从相应的链表中删除
    pipe=((struct pipe_struct*)inode->i_fs_priv_info);
    if(i==pipe->fd[0]) {
        //删除读端
        remove_task(pipe->pipe_r_next, inode);
    }else if(i==pipe->fd[1]){
        //删除写端
        remove_task(pipe->pipe_w_next,inode);
    }

    if(!pipe->pipe_r_next && !pipe->pipe_w_next){
        //没有读端也没有写端，则删除这个pipe，释放内存
//    if(atomic_test(&inode->i_used_count,0)) {
        if (inode->i_fs_priv_info) {
            OSFree(((struct pipe_struct *) (inode->i_fs_priv_info))->data);
            OSFree(inode->i_fs_priv_info);
            inode->i_fs_priv_info = NULL;
        }
//    }
        return ;
    }
}
struct file_operations pipe_fops={
    .open=pipe_open,
    .release=pipe_release,
    .read=pipe_read,
    .write=pipe_write
};
struct inode_operations pipe_iops={
        .default_file_ops=&pipe_fops,
};
