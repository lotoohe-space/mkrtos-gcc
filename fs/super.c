#include <mkrtos/fs.h>
#include <mkrtos/task.h>
#define SB_MAX_NUM 6

//ϵͳ���֧�ֵ�sb
struct super_block sb_list[SB_MAX_NUM]={0};
//ʹ�ñ�־λ
uint8_t sb_used_flag[SB_MAX_NUM]={0};

//û��ֱ�ӷ���NULL
struct super_block* get_empty_sb(void){
    int32_t i;
    for(i=0;i<SB_MAX_NUM;i++){
        if(sb_used_flag[i]==0){
            sb_used_flag[i]=1;
            return sb_list+i;
        }
    }
    return NULL;
}
//�ͷ�һ��sb
void free_sb(struct super_block* sb){
    int32_t i;
    for(i=0;i<SB_MAX_NUM;i++){
        if(sb_used_flag[i]==1){
           if(&(sb_list[i])==sb){
               sb_used_flag[i]=0;
               return ;
           }
        }
    }
}
//�ҵ�һ��ʹ���е�sb
struct super_block* find_sb(const dev_t d_no){
    int32_t i;
    for(i=0;i<SB_MAX_NUM;i++){
        if(sb_used_flag[i]==1){
            if(sb_list[i].s_dev_no==d_no){
                return &(sb_list[i]);
            }
        }
    }
    return NULL;
}
//��ȡָ���豸��sb
struct super_block* read_sb(dev_t dev_no,const char * fs_name,int32_t req_dev_no){
    struct super_block *sb;
    struct fs_type* ft;

    sb=find_sb(dev_no);
    if(sb){
        return sb;
    }
    sb=get_empty_sb();
    if(sb==NULL){
        return NULL;
    }
    ft=find_fs_type(fs_name);
    if(ft==NULL){
        return NULL;
    }
    sb->s_dev_no=dev_no;
    if(ft->alloc_sb(sb)==NULL){
        free_sb(sb);
        return NULL;
    }
    ft->read_sb(sb);
    sb->s_dev_no=dev_no;
    return sb;
}
int32_t sys_mount(void){

    return 0;
}
/**
 * ���ص�ǰ���̵ĸ��ļ�ϵͳ��ÿ������Ӧ�ö���Ҫ����һ��
 *
 */
void root_mount(struct _TaskBlock* task){
    uint32_t i=0;
    struct super_block* sb;
    for(i=0;i<fs_type_len;i++){

        sb = read_sb(root_dev_no,fs_type_list[i].f_name,fs_type_list[i].req_dev_no);
        if(!sb){
            continue;
        }

        task->root_inode = sb->root_inode;
        task->pwd_inode = sb->root_inode;

        return ;
    }
}


