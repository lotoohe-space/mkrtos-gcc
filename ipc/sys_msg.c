//
// Created by Administrator on 2022/2/25.
//
#include <sys/msg.h>
#include <sys/types.h>
#include <arch/atomic.h>
#include <mkrtos/task.h>
#include <errno.h>
#include <mkrtos/mem.h>
#include <arch/arch.h>
#define MSG_NUM 4
static struct msqid_ds msgid_ds_ls[MSG_NUM]={
        [0].msg_qbytes=2048
};
static Atomic_t msgid_ds_used[MSG_NUM]={0};
static Atomic_t msg_list_lk={0};

static Atomic_t msg_locks[MSG_NUM]={0};
struct s_msg{
    long mtype;
    char mtext[1];
};
#define DATA_SIZE 512
static void lock_msg(int id){
    while(!atomic_test_set(&msg_locks[id],1));
}
static void unlock_msg(int id){
    atomic_set(&msg_locks[id],0);
}

static struct msg* do_find_msg(struct msg* newmsg,int type){

    //type等于零，直接返回第一个
    if(!type){
        return newmsg;
    }else if(type>0) {
        while (newmsg) {
            if (newmsg->msg_type == type) {
                return newmsg;
            }
            newmsg = newmsg->next;
        }
    }else {
        //type<0
        //返回类型值最小的消息，这里直接返回第一个，因为插入按type大小插入的
        return newmsg;
    }
    return NULL;
}
//按照type的大小进行消息的插入
static void insert_msg(struct msg** head,struct msg* ins){
    struct msg *temp=*head;
    struct msg *prev=NULL;

    do{
        if(!temp && !prev){//没有任何消息
            *head=ins;
            break;
        }else if(!temp->next){ //最后一条消息
            if(!prev){//只有一条消息
                //头部插入
                ins->next=temp;
                *head=ins;
            }else{
                ins->next=temp;
                prev->next=ins;
            }
            break;
        }else{
            if(temp->msg_type>=ins->msg_type) {
                if(!prev){//只有一条消息
                    //头部插入
                    ins->next=temp;
                    *head=ins;
                }else{
                    ins->next=temp;
                    prev->next=ins;
                }
                break;
            }
        }
        prev=temp;
        temp=temp->next;
    }while(temp);
}
static void do_remove_msg(struct msg** head,struct msg* remmsg){
    struct msg *prev=NULL;
    struct msg *temp=*head;
    uint32_t t;
    while(temp){
        if(temp==remmsg) {
            if (prev==NULL) {
                //删除的第一个
                *head=temp->next;
                OSFree(temp->data);
                OSFree(temp);
                break;
            }else{
                prev->next=temp->next;
                break;
            }
        }
        prev=temp;
        temp=temp->next;
    }
}
static void do_remove_all_msg(struct msg* head){
    struct msg *temp=head;
    uint32_t t;
    while(temp){
        struct msg *next;
        next=temp->next;
        OSFree(temp->data);
        OSFree(temp);
        temp=next;
    }
}
static int32_t inner_find_msg(key_t key){
    int i;

    while(!atomic_test_set(&msg_list_lk,1));
    for(i=0;i<MSG_NUM;i++){
        if (!atomic_test(&msgid_ds_used[i],0)) {
            if (msgid_ds_ls[i].msg_perm.key == key) {
                atomic_set(&msg_list_lk, 0);
                return i;
            }
        }
    }
    atomic_set(&msg_list_lk,0);

    return -1;
}
static int32_t inner_msg_creat(key_t key,int flag){
    int i;

    while(!atomic_test_set(&msg_list_lk,1));
    for(i=0;i<MSG_NUM;i++){
        if (atomic_test_set(&msgid_ds_used[i],1)) {
            break;
        }
    }
    atomic_set(&msg_list_lk,0);
    if(i==MSG_NUM) {
        return -ENOSPC;
    }

    msgid_ds_ls[i].msg_perm.cgid=CUR_TASK->egid;
    msgid_ds_ls[i].msg_perm.cuid=CUR_TASK->euid;
    msgid_ds_ls[i].msg_perm.gid=CUR_TASK->rgid;
    msgid_ds_ls[i].msg_perm.uid=CUR_TASK->ruid;
    msgid_ds_ls[i].msg_perm.key=key;
    msgid_ds_ls[i].msg_perm.mode=flag;
    msgid_ds_ls[i].msg_perm.seq=0;

    msgid_ds_ls[i].msg_first=0;	/* first message on queue,unused  */
    msgid_ds_ls[i].msg_last=0;		/* last message in queue,unused */
    msgid_ds_ls[i].msg_stime=0;		/* last msgsnd time */
    msgid_ds_ls[i].msg_rtime=0;		/* last msgrcv time */
    msgid_ds_ls[i].msg_ctime=0;		/* last change time */
    msgid_ds_ls[i].msg_lcbytes=0;	/* Reuse junk fields for 32 bit */
    msgid_ds_ls[i].msg_lqbytes=0;	/* ditto */
    msgid_ds_ls[i].msg_cbytes=0;	/* current number of bytes on queue */
    msgid_ds_ls[i].msg_qnum=0;	/* number of messages in queue */
    msgid_ds_ls[i].msg_qbytes=2048;	/* max number of bytes on queue */
    msgid_ds_ls[i].msg_lspid=0;		/* pid of last msgsnd */
    msgid_ds_ls[i].msg_lrpid=0;		/* last receive pid */
    msgid_ds_ls[i].w_wait=0;		/* last receive pid */
    msgid_ds_ls[i].r_wait=0;		/* last receive pid */

    return i;
}

//唤醒队列中所有的任务
void msg_wake_up(struct msg_queue *queue//,int32_t msgtype
        ){
    uint32_t t;
    t=DisCpuInter();
    while(queue){
        if(queue->task){
            if(
                    //( msgtype<0 || queue->msgtype==msgtype)
                   // &&
                    queue->task->status==TASK_SUSPEND
                    ||queue->task->status==TASK_UNINTR
                    )
            {
                task_run_1(queue->task);
            }
        }
        queue=queue->next;
    }
    RestoreCpuInter(t);
}

//添加一个到等待队列中
void msg_add_wait_queue(struct msg_queue** queue,struct msg_queue* add_queue){
    uint32_t t;
    t=DisCpuInter();
    if(*queue==NULL){
        *queue=add_queue;
    }else{
        add_queue->next = (*queue);
        *queue=add_queue;
    }
    RestoreCpuInter(t);
}
//移除一个等待的
void msg_remove_wait_queue(struct msg_queue ** queue,struct msg_queue* add_queue){
    struct msg_queue *temp=*queue;
    struct msg_queue *prev=NULL;
    uint32_t t;
    if(!add_queue){
        return ;
    }
    t=DisCpuInter();
    while(temp){
        if(temp==add_queue) {
            if (prev==NULL) {
                //删除的第一个
                *queue=temp->next;
                break;
            }else{
                prev->next=temp->next;
                break;
            }
        }
        prev=temp;
        temp=temp->next;
    }
    RestoreCpuInter(t);
}
/**
 * 获取消息
 * @param key
 * @param flag
 * @return
 */
int sys_msgget(key_t key,int flag){
    int id;
    if(key==IPC_PRIVATE){
        id=inner_msg_creat(key,flag);
        return id;
    }else{
        id=inner_find_msg(key);
        if(!flag){
            if(id<0) {
                return -ENOENT;
            }
            return id;
        }else {
            //获取一个存在的
            if (flag & IPC_CREAT) {
                if (flag & IPC_EXCL) {
                    if(id>=0) {
                        return -ENOENT;
                    }
                    id=inner_msg_creat(key,flag);
                    return id;
                }
                if(id>=0){
                    return id;
                }
                id=inner_msg_creat(key,flag);
                return id;
            }
            return id;
        }
    }
    return -ENOMEM;
}
int sys_msgctl(int msgid,int cmd,struct msqid_ds *buf){
    struct msqid_ds *msq;
    if(msgid<0||msgid>=MSG_NUM){
        return -EINVAL;
    }
    if(!buf){
        return -EFAULT;
    }
    if(atomic_test(&msgid_ds_used[msgid],0)){
        return -EIDRM;
    }
    msq=&msgid_ds_ls[msgid];
    switch(cmd){
        case IPC_STAT:
            lock_msg(msgid);
            memcpy(buf,msq,sizeof(struct msqid_ds));
            lock_msg(msgid);
            break;
        case IPC_SET:
            if(CUR_TASK->euid==msq->msg_perm.cuid
            ||CUR_TASK->euid==msq->msg_perm.uid
            ||CUR_TASK->is_s_user
            ){
                lock_msg(msgid);
                msq->msg_perm.uid=buf->msg_perm.uid;
                msq->msg_perm.gid=buf->msg_perm.gid;
                msq->msg_perm.mode=buf->msg_perm.mode;
                msq->msg_qbytes=buf->msg_qbytes;
                unlock_msg(msgid);
            }
            break;
        case IPC_RMID:
            if(CUR_TASK->euid==msq->msg_perm.cuid
               ||CUR_TASK->euid==msq->msg_perm.uid
               ||CUR_TASK->is_s_user
                    ) {
                lock_msg(msgid);
                do_remove_all_msg(msgid_ds_ls[msgid].msg_first);
                //删除所有的数据
                atomic_set(&msgid_ds_used[msgid], 0);
                unlock_msg(msgid);
                //删除所有的消息
                //唤醒所有的在等待读或者写的任务，告诉他们任务被删除了
                msg_wake_up(msgid_ds_ls[msgid].r_wait);
                msg_wake_up(msgid_ds_ls[msgid].w_wait);
            }
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

int sys_msgrcv(int msgid,void *ptr,size_t nbytes,long type,int flag){
    struct s_msg *s_msg;
    struct msqid_ds *msq;
    struct msg* newmsg;
    struct msg * f_msg;

    if(msgid<0||msgid>=MSG_NUM){
        return -EINVAL;
    }
    if(!ptr){
        return -EFAULT;
    }
    if(atomic_test(&msgid_ds_used[msgid],0)){
        return -EIDRM;
    }
    s_msg=ptr;
    msq=&msgid_ds_ls[msgid];
    uint32_t rsize=nbytes;
    again_find:
    lock_msg(msgid);
    f_msg = do_find_msg(msq->msg_first,type);
    if(!f_msg){
        unlock_msg(msgid);
        if(flag&IPC_NOWAIT) {
            return -ENOMSG;
        }
        struct msg_queue msg_wait={CUR_TASK,NULL};
        msg_add_wait_queue(&msq->r_wait,&msg_wait);
        task_suspend();
        task_sche();
        task_run();
        msg_remove_wait_queue(&msq->r_wait,&msg_wait);

        //消息被删除了，则返回ERMID
        if(atomic_test(&msgid_ds_used[msgid],0)){
            return -ERMID;
        }
        if(CUR_TASK->sig_bmp[0]
        ||CUR_TASK->sig_bmp[1]
        ){
            return -EINTR;
        }
        goto again_find;
    }
    if(f_msg->msg_size>nbytes){
        if(!(flag&MSG_NOERROR)){
            unlock_msg(msgid);
            return -E2BIG;
        }
    }else{
        rsize=f_msg->msg_size;
    }
    //拷贝数据
    memcpy(s_msg->mtext,f_msg->data,rsize);
    //删除当前的
    do_remove_msg(&msq->msg_first,f_msg);
    msq->msg_cbytes-=f_msg->msg_size;
    msq->msg_qnum--;
    msq->msg_lrpid=CUR_TASK->PID;
    unlock_msg(msgid);
    //唤醒等待这个类型的消息
    msg_wake_up(msq->w_wait);
    return rsize;
}
int sys_msgsnd(int msgid,const void *ptr,size_t nbytes,int flag){
    struct s_msg *s_msg;
    struct msqid_ds *msq;
    struct msg* newmsg;

    if(msgid<0||msgid>=MSG_NUM){
        return -EINVAL;
    }
    if(!ptr){
        return -EFAULT;
    }
    if(atomic_test(&msgid_ds_used[msgid],0)){
        return -EIDRM;
    }
    s_msg=(struct s_msg *)ptr;
    if(s_msg->mtype<0){
        return -EINVAL;
    }
    msq=&msgid_ds_ls[msgid];

    if(nbytes>DATA_SIZE){
        if(!(flag&IPC_NOERROR)){
            return  -E2BIG;
        }
        //否则截断它
        nbytes=DATA_SIZE-nbytes;
    }
    //等待有空闲的空间写入数据
    again_e:
    lock_msg(msgid);
    if(nbytes+msq->msg_cbytes>msq->msg_qbytes){
        unlock_msg(msgid);
        //消息大小满了，等待
        if(flag&IPC_NOWAIT){
            return -EAGAIN;
        }
        //这里的msgtype设置为-1就好了，因为写不需要等待某个类型
        struct msg_queue msg_wait={CUR_TASK,NULL};

        msg_add_wait_queue(&msq->w_wait,&msg_wait);
        task_suspend();
        task_sche();
        task_run();
        msg_remove_wait_queue(&msq->w_wait,&msg_wait);

        //消息被删除了，则返回ERMID
        if(atomic_test(&msgid_ds_used[msgid],0)){
            return -ERMID;
        }
        if(CUR_TASK->sig_bmp[0]
           ||CUR_TASK->sig_bmp[1]){
            return -EINTR;
        }

        goto again_e;
    }
    unlock_msg(msgid);

    newmsg=OSMalloc(sizeof(struct msg));
    if(!newmsg){
        return -ENOMEM;
    }
    newmsg->data=OSMalloc(nbytes);
    if(!newmsg->data){
        OSFree(newmsg);
        return -ENOMEM;
    }
    newmsg->next=NULL;
    newmsg->msg_size=nbytes;
    newmsg->msg_type=s_msg->mtype;
    memcpy(newmsg->data,s_msg->mtext,nbytes);

    lock_msg(msgid);
    insert_msg(&msq->msg_first,newmsg);
//    //保存到开头
//    if(!msq->msg_first){
//        msq->msg_first=newmsg;
//        newmsg->next=NULL;
//    }else{
//        newmsg->next=msq->msg_first;
//        msq->msg_first=newmsg;
//    }
    msq->msg_qnum++;
    msq->msg_cbytes+=nbytes;
    msq->msg_lspid=CUR_TASK->PID;
    unlock_msg(msgid);
    //唤醒在等待读的队列
    msg_wake_up(msgid_ds_ls[msgid].r_wait);

    return nbytes;
}