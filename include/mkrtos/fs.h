//
// Created by Administrator on 2022/1/10.
//

#ifndef UNTITLED1_FS_H
#define UNTITLED1_FS_H

#include <errno.h>
#include <type.h>
#include <arch/atomic.h>

//��ǰ���̸�Ŀ¼INODE
#define ROOT_INODE ((struct inode*)(sysTasks.currentTask->root_inode))
//��ǰ���̹���Ŀ¼INODE
#define PWD_INODE ((struct inode*)(sysTasks.currentTask->pwd_inode))

struct file_operations;
struct inode_operations;
struct super_block;

//ȡ���ļ�����
#define FILE_TYPE(a) (((a)>>16)&0xffff)
//ȡ���ļ�ģʽ
#define FILE_MODE(a) ((a)&0xffff)

//�Ƿ�ΪĿ¼�ļ�
#define IS_DIR_FILE(a) (((a)>>16)==1)
//�Ƿ�Ϊ��ͨ�ļ�
#define IS_FILE(a) ((a)>>16)==1)

struct wait_queue;
//INode�ڵ�
typedef struct inode {
    //�ļ�������Ȩ��
    uint32_t i_type_mode;
    //�ļ���С
    uint32_t i_file_size;
    //�Լ���Inode����
    uint32_t i_no;
    //Ӳ������
    uint32_t i_hlink;
    //������豸�ļ�������������豸��
    uint32_t i_rdev_no;

    //�����������Ҫ�浽����

    //�豸����
    struct wait_queue *i_wait_q;
    //�򿪼���
//    Atomic_t i_open_count;
    //ʹ�ü���
    Atomic_t i_used_count;

    //���������inode
    Atomic_t i_lock;

    //�Ƿ��޸Ĺ�
    uint8_t i_dirt;
    struct super_block *i_sb;
    struct inode_operations* i_ops;
    //�ļ�ϵͳ��ţ�ͨ���ñ�Ų��ҵ���Ӧ���ļ�ϵͳ
    uint32_t i_fs_no;
    //�ļ�ϵͳӵ�е�˽����Ϣ
    void *i_fs_priv_info;
}*p_inode, inode;

//SupberBlock�ڵ�
typedef struct super_block {
    //ħ��
    uint32_t s_magic_num;
    //���С
    uint32_t s_bk_size;
    //���豸���豸��
    uint32_t s_dev_no;
    //inodeд�뵽���̵Ĵ�С
    uint16_t s_inode_size;

    //�ļ�ϵͳ��ţ�ͨ���ñ�Ų��ҵ���Ӧ���ļ�ϵͳ
    uint32_t s_fs_no;
    //�ļ�ϵͳ���ڵ��inode
    struct inode *root_inode;
    //�Ƿ��޸Ĺ�
    uint8_t s_dirt;
    //super��������
    struct super_operations* s_ops;
    //�ļ�ϵͳӵ�е�˽����Ϣ
    void *s_sb_priv_info;
}*p_super_block, super_block;


struct file {
    uint8_t f_mode;		    /* �ļ�������ʱ�������ļ���Ȩ�� */
    int32_t f_ofs;            /* �ļ���дƫ���� */
//    uint32_t f_count;           /*���file��ʹ���˶��ٴ�,��ʱ�ò���*/
    unsigned short f_flags; /* ��ʲô���ķ�ʽ���ļ�����ֻ����ֻд�ȵ� */
    struct inode * f_inode;		/* �ļ���Ӧ��inode */
    struct file_operations * f_op; /*�ļ���Ӧ�Ĳ�����*/
    uint8_t used;   /*�Ƿ�ʹ�ñ��*/
};

struct dirent
{
    long d_ino; /*�����ڵ�� */
    int32_t d_off; /*��Ŀ¼�ļ��е�ƫ�� */
    uint16_t d_file_name_len; /*�ļ����� */
//    unsigned char d_type; /*�ļ����� */
    char d_name [128+1]; /*�ļ������128�ַ� */
};

/* ��inode��Ӧ�ļ��Ĳ���
 */
struct file_operations {
    int (*lseek) (struct inode *, struct file *, off_t, int);
    int (*read) (struct inode *, struct file *, char *, int);
    int (*write) (struct inode *, struct file *, char *, int);
    int (*readdir) (struct inode *, struct file *, struct dirent *, int);
    int (*select) (struct inode *, struct file *, int, uint32_t *);
    int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
    int (*mmap) (struct inode *, struct file *, unsigned long, size_t, int, unsigned long);
    int (*open) (struct inode *, struct file *);
    void (*release) (struct inode * ino, struct file * f);
    int (*fsync) (struct inode *, struct file *);
};

/* ��inode�Ĳ���
 */
struct inode_operations {
    struct file_operations * default_file_ops;
    int (*create) (struct inode *dir,const char *name,int len,int mode,struct inode **result);
    //����ƶ�
    int (*lookup) (struct inode * dir,const char * file_name,int len,struct inode ** ret_inode);
    int (*link) (struct inode *,struct inode *,const char *,int);
    int (*unlink) (struct inode *,const char *,int);
    int (*symlink) (struct inode *,const char *,int,const char *);
    int (*mkdir) (struct inode *,const char *,int,int);
    int (*rmdir) (struct inode *,const char *,int);
    int (*mknod) (struct inode *,const char *,int,int,int);
    int (*rename) (struct inode *,const char *,int,struct inode *,const char *,int);
    int (*readlink) (struct inode *,char *,int);
    int (*follow_link) (struct inode *,struct inode *,int,int,struct inode **);
    int (*bmap) (struct inode *,int);
    void (*truncate) (struct inode *);
    int (*permission) (struct inode *, int);
};

struct super_operations {
    //����ָ���ļ�ϵͳ��inode
    struct inode * (*alloc_inode)(struct inode * p_inode);
    //�ͷ�ָ���ļ�ϵͳ��inode
    int32_t (*free_inode)(struct inode *p_inode);

    int32_t (*read_inode) (struct inode * p_r_ino);
    int (*notify_change) (int flags, struct inode *);
    void (*write_inode) (struct inode *);
    void (*put_inode) (struct inode *);
    void (*put_super) (struct super_block *);
    void (*write_super) (struct super_block *);
    void (*statfs) (struct super_block *, struct statfs *);
    int (*remount_fs) (struct super_block *, int *, char *);
};

struct fs_type{
    //�ļ�ϵͳ������
    const char* f_name;
    //�Ƿ���Ҫ�豸��
    uint8_t req_dev_no;
    //������ļ�ϵͳ�Ĳ�����
    struct super_block * (*read_sb)(struct super_block * sb);
    struct super_block* (*alloc_sb)(struct super_block* sb);
    void (*free_sb)(struct super_block* sb);
};

//fs_type.c�ļ�ϵͳ�������
extern struct fs_type fs_type_list[];
extern int32_t fs_type_len;
struct fs_type *find_fs_type(const char *fs_name);

//open.c
//

//inode.c ��Ŀ¼���豸��
void lose_inode(struct inode* p_inode);
struct inode* get_empty_inode(void);
struct inode* geti(struct super_block* p_sb,ino_t ino);
int32_t puti(struct inode* put_inode);

void wait_on_inode(struct inode* inode);
void lock_inode(struct inode* inode);
void unlock_inode(struct inode* inode);


//namei.c
struct inode* _open_namei(const char*file_path);
struct inode* open_namei(const char* file_path,int32_t flags,int32_t mode);

//�黺�����
struct bk_cache {
    //���Inx
    uint32_t bk_no;
    uint32_t bk_size;
    //�����������Ҫ���棬��ΪNull
    uint8_t* cache;
    //usedCount���ڼ���ÿ鱻ʹ�ö��ٴΣ�ÿ�λ�����ʱ�����߳�ʹ�ô�����С���Ǹ�
//    uint32_t used_count;
    uint32_t sem_lock;
    //������� 1bitд�� 2bit��ȡ 7bit��ʹ��
    uint8_t flag;
};
////////

//dev.c
extern dev_t root_dev_no;
extern struct inode_operations chrdev_inode_operations;
extern struct inode_operations blkdev_inode_operations;
struct bk_operations{
    /**
     * ��ȡ��
     * @param bk_no ���
     * @param data ��Ŷ�ȡ������
     * @return
     */
    int32_t (*read_bk)(uint32_t bk_no,uint8_t *data);
    int32_t (*write_bk)(uint32_t bk_no,uint8_t *data);
    int32_t (*erase_bk)(uint32_t bk_no);
};
uint32_t get_bk_size(dev_t major_no) ;
struct bk_operations* get_bk_ops(dev_t major_no);
uint32_t get_bk_count(dev_t major_no);
struct dev_cache* get_bk_dev_cache(dev_t major_no,uint32_t *res_cache_len);
int32_t reg_bk_dev(
        dev_t major_no,
        const char* name ,
        struct bk_operations *bk_ops,
        uint32_t bk_count,
        uint32_t cache_len,
        uint32_t bk_size
);
int32_t unreg_bk_dev(dev_t major_no,const char* name);
struct file_operations* get_ch_dev_ops(dev_t major_no);
int32_t reg_ch_dev(dev_t major_no,const char* name ,struct file_operations *d_fops);
int32_t unreg_ch_dev(dev_t major_no,const char* name);

void devs_init(void);
/////////

//bk.c
int32_t request_bk_no(dev_t dev_no);
int32_t alloc_bk_no();

//super.c
struct super_block* get_empty_sb(void);
void free_sb(struct super_block* sb);


//super.c
struct task;
void root_mount(struct task* task);

/**
 * @breif: ��ͬ������Setion�Ķ���
 */
#ifndef SECTION
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && __ARMCC_VERSION >= 6000000)
#define SECTION(x)                  __attribute__((section(x)))
#elif defined(__ICCARM__) || defined(__ICCRX__)
#define SECTION(x)                  @ x
#elif defined(__GNUC__)
#define SECTION(x)                  __attribute__((section(x)))
#else
#define SECTION(x)
#endif
#endif

struct dev_reg{
    int32_t (*init)(void);
    int32_t (*exit)(void);
};
/**
 * @breif dev���ٳ�ʼ��
 */
#define DEV_BK_EXPORT(_initFun,_exitFun,_name) \
        const struct dev_reg \
        dev_reg_##_name SECTION("DevRegList") = {\
			.init=_initFun,\
			.exit=_exitFun\
		}


#endif //UNTITLED1_FS_H
