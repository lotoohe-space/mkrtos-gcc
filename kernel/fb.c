//
// Created by Administrator on 2022/4/11.
//
#include <mkrtos/fb.h>
#include <mkrtos/fs.h>
#include <mkrtos/dev.h>

#define FB_INFO_NUM 2

static struct fb_info fb_infos[FB_INFO_NUM]={0};
static int fb_info_used[FB_INFO_NUM]={0};

#if FB_INFO_NUM<1
#error FB NUM must be over 1!
#endif

static int open(struct inode * inode, struct file * fp){
    int major;
    int minor;
    int ret=-ENOSYS;
    major=MAJOR(inode->i_rdev_no);
    minor=MINOR(inode->i_rdev_no);
    if(major!=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(minor<0||minor>=FB_DEV_MAJOR){
        return -ENODEV;
    }

    if(fb_infos[minor]._fb_ops
    &&fb_infos[minor]._fb_ops->fb_open
    ){
       ret=fb_infos[minor]._fb_ops->fb_open(&fb_infos[minor],0);
    }
    if(ret>=0){
        fb_info_used[minor]=1;
    }
    return ret;
}
static void release(struct inode * inode, struct file * fp){
    int major;
    int minor;
    major=MAJOR(inode->i_rdev_no);
    minor=MINOR(inode->i_rdev_no);
    if(major!=FB_DEV_MAJOR){
        return ;
    }
    if(minor<0||minor>=FB_DEV_MAJOR){
        return ;
    }
    if(!fb_info_used[minor]){
        return ;
    }
    if(fb_infos[minor]._fb_ops
       && fb_infos[minor]._fb_ops->fb_release
            ){
        fb_infos[minor]._fb_ops->fb_release(&fb_infos[minor],0);
    }
    fb_info_used[minor]=0;
}

static int read(struct inode *inode, struct file *fp, char * data, int cn){
    int major;
    int minor;
    int ret=-ENOSYS;
    major=MAJOR(inode->i_rdev_no);
    minor=MINOR(inode->i_rdev_no);
    if(major!=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(minor<0||minor>=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(!fb_info_used[minor]){
        return -ENODEV;
    }
    if(fb_infos[minor]._fb_ops
       && fb_infos[minor]._fb_ops->fb_read
            ){
        ret=fb_infos[minor]._fb_ops->fb_read(&fb_infos[minor],data,cn,&(fp->f_ofs));
    }
    return ret;
}
static int write(struct inode *inode, struct file *fp, char * data, int cn){
    int major;
    int minor;
    int ret=-ENOSYS;
    major=MAJOR(inode->i_rdev_no);
    minor=MINOR(inode->i_rdev_no);
    if(major!=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(minor<0||minor>=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(!fb_info_used[minor]){
        return -ENODEV;
    }
    if(fb_infos[minor]._fb_ops
       && fb_infos[minor]._fb_ops->fb_write
            ){
        ret=fb_infos[minor]._fb_ops->fb_write(&fb_infos[minor],data,cn,&(fp->f_ofs));
    }
    return ret;
}
static int ioctl(struct inode *inode, struct file *fp, unsigned int cmd, unsigned long arg){
    int major;
    int minor;
    struct fb_info *fb;
    int ret=-ENOSYS;
    major=MAJOR(inode->i_rdev_no);
    minor=MINOR(inode->i_rdev_no);
    if(major!=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(minor<0||minor>=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(!fb_info_used[minor]){
        return -ENODEV;
    }
    if(fb_infos[minor]._fb_ops
       &&fb_infos[minor]._fb_ops->fb_ioctl
            ){
        ret=fb_infos[minor]._fb_ops->fb_ioctl(&fb_infos[minor],cmd,arg);
    }
    return ret;
}
/**
 * 该函数可以将屏幕缓冲映射到内存
 * @param inode
 * @param fp
 * @param addr
 * @param len
 * @param mask
 * @param off
 * @return
 */
static int mmap(struct inode * inode, struct file * fp, unsigned long addr, size_t len , int mask, unsigned long off){
    int major;
    int minor;
    int ret=-ENOSYS;
    major=MAJOR(inode->i_rdev_no);
    minor=MINOR(inode->i_rdev_no);
    if(major!=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(minor<0||minor>=FB_DEV_MAJOR){
        return -ENODEV;
    }
    if(!fb_info_used[minor]){
        return -ENODEV;
    }
    if(fb_infos[minor]._fb_ops
       &&fb_infos[minor]._fb_ops->fb_mmap
            ){
        ret=fb_infos[minor]._fb_ops->fb_mmap(&fb_infos[minor],addr,len,mask,off);
    }

    return ret;
}
static int mumap(struct inode * inode,void * start ,size_t len){
    return 0;
}

static int fsync (struct inode *inode, struct file *fp){
    return -ENOSYS;
}
static struct file_operations f_ops={
        .open=open,
        .release=release,
        .read=read,
        .write=write,
        .ioctl=ioctl,
        .mmap=mmap,
        .mumap=mumap,
        .fsync=fsync
};
static int fb_init(void){
    if(reg_ch_dev(FB_DEV_MAJOR,
                  "fb",
                  &f_ops
    )<0){
        return -1;
    }
    return 0;
}
static int fb_exit(void){
    unreg_ch_dev(FB_DEV_MAJOR,"fb");
    return 0;
}
DEV_BK_EXPORT(fb_init,fb_exit,fb);
