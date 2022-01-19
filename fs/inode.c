//
// Created by Administrator on 2022/1/12.
//

#include <mkrtos/fs.h>
#include <mkrtos/mem.h>
#include <ipc/ipc.h>


//ϵͳ��֧�ֵ����inode����
#define INODE_NUM 32
struct inode inode_ls[INODE_NUM];

//inode �����ź���id
#define INODE_SEM_ID 1
//���е�inode����
Atomic_t inode_free_num={
        .counter=INODE_NUM
};
//�ź�����id��Ĭ��-1������û�У��ȴ�����
sem_t sem_inode_ls=-1;

/**
 * ��ס���inode���õ�ʱ�����
 * @param p_inode
 */
void locki(struct inode* p_inode){
    atomic_set(&(p_inode->i_lock),1);
}
/**
 * ����
 * @param p_inode
 */
void unlocki(struct inode* p_inode){
    atomic_set(&(p_inode->i_lock),0);
}
/**
 * ��ȡһ���յ�inode
 * @return
 */
struct inode* get_empty_inode(void){
    uint32_t i;
    if(sem_inode_ls==-1){
        sem_inode_ls=sem_get(INODE_SEM_ID);
    }
    if(atomic_read(&inode_free_num)==0){
        //��������Ҳ�����Ӧ����ϵͳ���ߵȴ��ͷ��źŵ���
        if(sem_take(sem_inode_ls)<0){
            //zhangzheng �����ܵ�������
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
    //��������Ҳ�����Ӧ����ϵͳ���ߵȴ��ͷ��źŵ���
    if(sem_take(sem_inode_ls)<0){
        //zhangzheng �����ܵ�������
        while(1);
    }
    //���²��ҿ��õ�
    goto again;
}
/**
 * ����һ��inode
 * @param p_inode
 */
void lose_inode(struct inode* p_inode){
    //�����������
    atomic_test_dec_nq(&(p_inode->i_used_count));
    if(atomic_read(&(p_inode->i_used_count))==0){
        //�ͷŵȴ��Ľ���
        sem_release(sem_inode_ls);
    }
    atomic_inc(&inode_free_num);
}

/**
 * ��ȡinode
 * @param p_sb
 * @param ino
 * @return
 */
struct inode* geti(struct super_block* p_sb,ino_t ino){

    struct inode* r_inode= NULL;
    r_inode=get_empty_inode();

    atomic_set(&(r_inode->i_open_count),0);
    //ʹ�ü���
    atomic_set(&(r_inode->i_used_count),1);
    //���������inode
    atomic_set(&(r_inode->i_lock),0);
    //������
    r_inode->i_sb=p_sb;
    r_inode->i_no=ino;
    r_inode->i_hlink=0;
    //����һ��inode
    if(p_sb->s_ops->alloc_inode(r_inode) == NULL){
        lose_inode(r_inode);
        //����ʧ��
        return NULL;
    }

    //��ȡinode
    if(p_sb->s_ops->read_inode(r_inode)<0){
        p_sb->s_ops->free_inode(r_inode);
        lose_inode(r_inode);
    }

    return r_inode;
}
/**
 * �ͷ�һ��inode���õ�һ�����е�inode
 * @param put_inode
 * @return
 */
int32_t puti(struct inode* put_inode){

    //wait_inode();

    if(!atomic_read(&( put_inode->i_used_count))){
        //�ͷŵĿյ�inode������һ�������
        return -1;
    }

    //����1��ֱ�Ӽ�1������
    if(atomic_read(&( put_inode->i_used_count))>1){
        lose_inode(put_inode);
        return 0;
    }

    //����1����Ҫд��inode
    if(put_inode
        &&put_inode->i_sb
        &&put_inode->i_sb->s_ops
        &&put_inode->i_sb->s_ops->put_inode
    ){
        //Ӳ����������������ͷ����inode
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


