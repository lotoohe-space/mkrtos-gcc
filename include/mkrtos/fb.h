//
// Created by Administrator on 2022/4/11.
//

#ifndef UNTITLED1_FB_H
#define UNTITLED1_FB_H
#include <sys/types.h>
#include <ipc/spin_lock.h>
#define COLOR_MODE 0 //RGB565
#define COLOR_MODE 1 //BGR565
#define COLOR_MODE 2 //RGB888
#define COLOR_MODE 3 //ARGB888

struct fb_info;


struct fb_ops {
    /* open/release and usage marking */
    struct module *owner;
    int (*fb_open)(struct fb_info *info, int user);
    int (*fb_release)(struct fb_info *info, int user);

    /* For framebuffers with strange non linear layouts or that do not
     * work with normal memory mapped access
     */
    ssize_t (*fb_read)(struct fb_info *info, char *buf,size_t count, loff_t *ppos);
    ssize_t (*fb_write)(struct fb_info *info, const char *buf, size_t count, loff_t *ppos);

    /* checks var and eventually tweaks it to something supported,
     * DO NOT MODIFY PAR */
//    int (*fb_check_var)(struct fb_var_screeninfo *var, struct fb_info *info);

    /* set the video mode according to info->var */
//    int (*fb_set_par)(struct fb_info *info);

    /* set color register */
//    int (*fb_setcolreg)(unsigned regno, unsigned red, unsigned green,
//                        unsigned blue, unsigned transp, struct fb_info *info);

    /* set color registers in batch */
//    int (*fb_setcmap)(struct fb_cmap *cmap, struct fb_info *info);

    /* blank display */
//    int (*fb_blank)(int blank, struct fb_info *info);

    /* pan display */
//    int (*fb_pan_display)(struct fb_var_screeninfo *var, struct fb_info *info);

    /* Draws a rectangle */
//    void (*fb_fillrect) (struct fb_info *info, const struct fb_fillrect *rect);
    /* Copy data from area to another */
//    void (*fb_copyarea) (struct fb_info *info, const struct fb_copyarea *region);
    /* Draws a image to the display */
//    void (*fb_imageblit) (struct fb_info *info, const struct fb_image *image);

    /* Draws cursor */
//    int (*fb_cursor) (struct fb_info *info, struct fb_cursor *cursor);
//
//    /* Rotates the display */
//    void (*fb_rotate)(struct fb_info *info, int angle);
//
//    /* wait for blit idle, optional */
//    int (*fb_sync)(struct fb_info *info);

    /* perform fb specific ioctl (optional) */
    int (*fb_ioctl)(struct fb_info *info, unsigned int cmd,
                    unsigned long arg);

    /* Handle 32bit compat ioctl (optional) */
//    int (*fb_compat_ioctl)(struct fb_info *info, unsigned cmd,
//                           unsigned long arg);

    /* perform fb specific mmap */
    int (*fb_mmap)(struct fb_info *info,  unsigned long addr, size_t len , int mask, unsigned long off);

    /* get capability given var */
//    void (*fb_get_caps)(struct fb_info *info, struct fb_blit_caps *caps,
//                        struct fb_var_screeninfo *var);

    /* teardown any resources to do with this framebuffer */
    void (*fb_destroy)(struct fb_info *info);

    /* called at KDB enter and leave time to prepare the console */
    int (*fb_debug_enter)(struct fb_info *info);
    int (*fb_debug_leave)(struct fb_info *info);
};
/**
 * 显示参数信息
 */
struct fb_info{
    struct fb_ops *_fb_ops;
    void* addr;//显示地址
    int width;//宽
    int height;//高
    int color_mode;//颜色模式
    int show_ori;//显示方向

    spinlock_handler  lock;
};

#endif //UNTITLED1_FB_H
