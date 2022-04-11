//
// Created by Administrator on 2022/4/11.
//

#include <mkrtos/fb.h>
#include <errno.h>

static int fb_open(struct fb_info *info, int user){

    return -ENOSYS;
}
static int fb_release(struct fb_info *info, int user){
    return -ENOSYS;
}

/* For framebuffers with strange non linear layouts or that do not
 * work with normal memory mapped access
 */
static ssize_t fb_read(struct fb_info *info, char *buf,size_t count, loff_t *ppos){
    return -ENOSYS;
}
static ssize_t fb_write(struct fb_info *info, const char *buf, size_t count, loff_t *ppos){
    return -ENOSYS;
}
static int fb_mmap(struct fb_info *info, struct vm_area_struct *vma){
    return -ENOSYS;
}