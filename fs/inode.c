//
// Created by Administrator on 2022/1/12.
//

#include <mkrtos/fs.h>
#include <mkrtos/mem.h>
#include <ipc/ipc.h>


//系统内支持的最大inode数量
#define INODE_NUM 32
struct inode inode_ls[INODE_NUM];

//inode 链表信号量id
#define INODE_SEM_ID 1
//空闲的inode数量
Atomic_t inode_free_num={
        .counter=INODE_NUM
};
//信号量的id，默认-1，代表没有，等待创建
sem_t sem_inode_ls=-1;

/**
 * 锁住这个inode，用的时候才锁
 * @param p_inode
 */
void locki(struct inode* p_inode){
    atomic_set(&(p_inode->i_lock),1);
}
/**
 * 解锁
 * @param p_inode
 */
void unlocki(struct inode* p_inode){
    atomic_set(&(p_inode->i_lock),0);
}
/**
 * 获取一个空的inode
 * @return
 */
struct inode* get_empty_inode(void){
    uint32_t i;
    if(sem_inode_ls==-1){
        sem_inode_ls=sem_get(INODE_SEM_ID);
    }
    if(atomic_read(&inode_free_num)==0){
        //如果这里找不到，应该让系统休眠等待释放信号到来
        if(sem_take(sem_inode_ls)<0){
            //zhangzheng 不可能到这里来
            while(1);
        }
    }
    again:
    for(i=0;i<INODE_NUM;i++){
        if(atomic_test_inc(&(inode_ls[i].i_used_count))){
            atomic_dec(&inode_free_num);
            return &inode_ls;
        }
    }
    //如果这里找不到，应该让系统休眠等待释放信号到来
    if(sem_take(sem_inode_ls)<0){
        //zhangzheng 不可能到这里来
        while(1);
    }
    //重新查找可用的
    goto again;
}
/**
 * 放弃一个inode
 * @param p_inode
 */
void lose_inode(struct inode* p_inode){
    //不等于零则减
    atomic_test_dec_nq(&(p_inode->i_used_count));
    if(atomic_read(&(p_inode->i_used_count))==0){
        //释放等待的进程
        sem_release(sem_inode_ls);
    }
    atomic_inc(&inode_free_num);
}

/**
 * 获取inode
 * @param p_sb
 * @param ino
 * @return
 */
struct inode* geti(struct super_block* p_sb,ino_t ino){

    struct inode* r_inode= NULL;
    r_inode=get_empty_inode();

    atomic_set(&(r_inode->i_open_count),0);
    //使用计数
    atomic_set(&(r_inode->i_used_count),1);
    //用来锁这个inode
    atomic_set(&(r_inode->i_lock),0);
    //填充参数
    r_inode->i_sb=p_sb;
    r_inode->i_no=ino;
    r_inode->i_hlink=0;
    //申请一个inode
    if(p_sb->s_ops->alloc_inode(r_inode) == NULL){
        lose_inode(r_inode);
        //申请失败
        return NULL;
    }

    //读取inode
    if(p_sb->s_ops->read_inode(r_inode)<0){
        p_sb->s_ops->free_inode(r_inode);
        lose_inode(r_inode);
    }

    return r_inode;
}
/**
 * 释放一个inode，得到一个空闲的inode
 * @param put_inode
 * @return
 */
int32_t puti(struct inode* put_inode){

    //wait_inode();

    if(!atomic_read(&( put_inode->i_used_count))){
        //释放的空的inode，这算一个错误吧
        return -1;
    }

    //大于1则直接减1就行了
    if(atomic_read(&( put_inode->i_used_count))>1){
        lose_inode(put_inode);
        return 0;
    }

    //等于1则需要写入inode
    if(put_inode
        &&put_inode->i_sb
        &&put_inode->i_sb->s_ops
        &&put_inode->i_sb->s_ops->put_inode
    ){
        //硬链接数等于零才能释放这个inode
        if(put_inode->i_hlink==0) {
            put_inode->i_sb->s_ops->put_inode(put_inode);
        }
    }

    if(put_inode
       &&put_inode->i_sb
       &&put_inode->i_sb->s_ops
       &&put_inode->i_sb->s_ops->write_inode
    ){
        put_inode->i_sb->s_ops->write_inode(put_inode);
    }

    lose_inode(put_inode);
    return 0;
}


