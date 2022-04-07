//
// Created by Administrator on 2022/4/7.
//

//
// Created by Administrator on 2022/1/16.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <bsp/lib/sys.h>
#include <bsp/sd_card.h>
#define TOTAL_KB        (512)
#define CODE_KB         (256)
#define FLASH_DEV_NO 1
#define FLASH_BK_CACHE_LEN 3

#define BK_SIZE 512
#define BK_COUNT 512
static int32_t open_bk(uint32_t bk_no) {
    if (SD_Init() != SD_OK) {
        return -1;
    }
    //设置块设备参数
    if (bk_dev_reg_param(
            FLASH_DEV_NO,
            SDCardInfo.CardCapacity / 512,
            FLASH_BK_CACHE_LEN,
            SDCardInfo.CardBlockSize) < 0) {
        return -1;
    }
    return 0;
}
static int32_t read_bk(uint32_t bk_no,uint8_t *data){
    if(SD_ReadDisk(data,bk_no,1)!=SD_OK){
        return -1;
    }
    return 0;
}
static int32_t write_bk(uint32_t bk_no,uint8_t *data){
    if(SD_WriteDisk(data,bk_no,1)!=SD_OK){
        return -1;
    }
    return 0;
}
static int32_t erase_bk(uint32_t bk_no){
   return 0;
}
static void release_bk(uint32_t bk_no){
    bk_dev_unreg_param(bk_no);
}
static struct bk_operations bk_flash={
        .open=open_bk,
        .read_bk=read_bk,
        .write_bk=write_bk,
        .erase_bk=erase_bk,
        .release=release_bk
};
static struct file_operations emmc_ops={
        .open=bk_file_open,
        .read=bk_file_read,
        .write=bk_file_write,
        .fsync=bK_file_fsync,
        .release=bk_file_release
};
static dev_t used_dev_no=FLASH_DEV_NO;
#define DEV_NAME "mmc"
/**
 * 注册设备驱动，注册成功返回设备驱动号
 * @return
 */
int32_t mmc_init(void){

    if(request_bk_no(FLASH_DEV_NO)<0){
        return -1;
    }

    if(reg_bk_dev(FLASH_DEV_NO,DEV_NAME,
                  &bk_flash,
                  &emmc_ops
//                  ,
//                  BK_COUNT,
//                  FLASH_BK_CACHE_LEN,BK_SIZE
    )<0){
        return -1;
    }

    return used_dev_no;
}
int32_t mmc_exit(void){

    unreg_bk_dev(used_dev_no,DEV_NAME);

}
DEV_BK_EXPORT(mmc_init,mmc_exit,mmc);
