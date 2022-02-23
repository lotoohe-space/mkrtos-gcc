//
// Created by zz on 2022/2/23.
//
#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#include <fcntl.h>
#include <mkrtos/mem.h>

extern struct inode_operations pipe_iops;

struct inode * get_pipe_inode(void){
    struct inode *r_inode;
    r_inode=get_empty_inode();
    //初始化pipe_inode信息
    //使用计数
    atomic_set(&(r_inode->i_used_count),1);
    atomic_set(&(r_inode->i_lock),0);
    r_inode->i_file_size=0;
    r_inode->i_wait_q=NULL;
    r_inode->i_type_mode= MK_MODE(S_IFIFO,0777);
    //填充参数
    r_inode->i_sb=NULL;
    //这里设置一个无效的inode号码，这样不会与文件系统的iode号码冲突
    r_inode->i_no=-1;
    r_inode->i_hlink=0;
    r_inode->i_fs_priv_info=NULL;
    r_inode->i_ops=&pipe_iops;

    return r_inode;
}

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

    fd[0]=fds[0];
    fd[1]=fds[1];

    return 0;
}

struct pipe_struct{
    uint8_t *data;
    uint32_t data_len;
    uint32_t rear;
    uint32_t front;
    uint32_t lock;
    struct task* r_wait_tk;
    struct task* w_wait_tk;
};
#define PIPE_LEN(a) ((a->rear-a->front+a->data_len)%a->data_len)
#define MIN(a,b) ((a)<(b)?(a):(b))
#define FIFO_SIZE 512

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
        //没有数据则等待
        pipe->r_wait_tk=CUR_TASK;
        task_suspend();
        task_sche();
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
        task_run_1(pipe->w_wait_tk);
        if(read_inx>=cn){
            return read_inx;
        }else{
            read_cn=cn-read_inx;
            //没有读够，继续等待
            goto again;
        }
    }else{
        pipe->r_wait_tk=CUR_TASK;
        task_suspend();
        task_sche();
        goto again_lock;
    }
    atomic_set(&pipe->lock,0);
    return 0;
}
static int pipe_write (struct inode *inode, struct file *fp, char *buf, int cn){
    struct pipe_struct *pipe;
    int write_cn=cn;
    int write_inx=0;
    pipe=inode->i_fs_priv_info;
    again:
    if((pipe->rear+1)%pipe->data_len==pipe->front) {
        //满了，等待
        pipe->w_wait_tk=CUR_TASK;
        task_suspend();
        task_sche();
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
        task_run_1(pipe->r_wait_tk);
        if(write_inx>=cn){
            return write_inx;
        }else{
            write_cn=cn-write_inx;
            goto again;
        }
    }else{
        pipe->w_wait_tk=CUR_TASK;
        task_suspend();
        task_sche();
        goto again_lock;
    }
    atomic_set(&pipe->lock,0);
    return 0;
}
static int pipe_open (struct inode * inode, struct file * fp){
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
static void pipe_release (struct inode * inode, struct file * fp){
    if(inode->i_fs_priv_info){
        OSFree(((struct pipe_struct*)(inode->i_fs_priv_info))->data);
        OSFree(inode->i_fs_priv_info);
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