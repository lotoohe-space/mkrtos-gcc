//
// Created by Administrator on 2022/1/16.
//
#include <type.h>
#include <mkrtos/fs.h>
#include <bsp/lib/sys.h>
#include <mkrtos/dev.h>
#define TOTAL_KB        (512)
#define CODE_KB         (256)
#define REMAIN_BK       ((TOTAL_KB-CODE_KB)>>1)

#define CR_LOCK_Set              ((uint32_t)0x00000080)

#define FLASH_KEY1               ((uint32_t)0x45670123)
#define FLASH_KEY2               ((uint32_t)0xCDEF89AB)

#define FLASH_W_BASE_ADDR        (0x8000000+CODE_KB*1024)

#define FLASH_BK_CACHE_LEN 3

#define BK_SIZE 2048
#define BK_COUNT REMAIN_BK

void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u16 i;
    for(i=0;i<NumToWrite;i++)
    {
        FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
        WriteAddr+=2;//地址增加2.
    }
}
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(vu16*)faddr;
}
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
    u16 i;
    for(i=0;i<NumToRead;i++)
    {
        pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
        ReadAddr+=2;//偏移2个字节.
    }
}
//int32_t init_bk(void){
//    return 0;
//}
static int32_t read_bk(uint32_t bk_no,uint8_t *data){
    STMFLASH_Read(bk_no*BK_SIZE+FLASH_W_BASE_ADDR,
                  (uint16_t*)data,
                  BK_SIZE/2);
    return 0;
}
static int32_t write_bk(uint32_t bk_no,uint8_t *data){
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    STMFLASH_Write_NoCheck(bk_no*BK_SIZE+FLASH_W_BASE_ADDR,
                           (uint16_t*)data,
                           BK_SIZE/2);//写已经擦除了的,直接写入扇区剩余区间.
    FLASH->CR |= CR_LOCK_Set;
}
static int32_t erase_bk(uint32_t bk_no){
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    FLASH_ErasePage(bk_no*BK_SIZE+FLASH_W_BASE_ADDR);
    FLASH->CR |= CR_LOCK_Set;
}

static struct bk_operations bk_flash={
//    .init_bk=init_bk,
    .read_bk=read_bk,
    .write_bk=write_bk,
    .erase_bk=erase_bk
};
static struct file_operations bk_ops={
        .open=bk_file_open,
        .read=bk_file_read,
        .write=bk_file_write,
        .fsync=bK_file_fsync,
        .release=bk_file_release
};
dev_t used_dev_no=BK_FLASH;
#define DEV_NAME "flash"
/**
 * 注册设备驱动，注册成功返回设备驱动号
 * @return
 */
int32_t bk_flash_init(void){

    if(request_bk_no(BK_FLASH)<0){
//        if((used_dev_no=alloc_bk_no())<0){
//            return -1;
//        }
        sys_mknod("/dev/flash",0777|(3<<16),BK_FLASH);
        return -1;
    }

    if(reg_bk_dev(BK_FLASH,DEV_NAME,
                  &bk_flash,
                  &bk_ops
//                  ,
//                  BK_COUNT,
//                  FLASH_BK_CACHE_LEN,BK_SIZE
                  )<0){
        return -1;
    }
    bk_dev_reg_param(BK_FLASH,BK_COUNT,FLASH_BK_CACHE_LEN,BK_SIZE);
    return used_dev_no;
}
int32_t bk_flash_exit(void){

    unreg_bk_dev(used_dev_no,DEV_NAME);

}
DEV_BK_EXPORT(bk_flash_init,bk_flash_exit,flash);
