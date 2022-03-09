//
// Created by Administrator on 2022/3/9.
//

#include <mkrtos/fs.h>

int sys_sync(void){
    sync_all_sb();
    sync_all_inode();
    sync_all_bk_dev();
    return 0;
}

