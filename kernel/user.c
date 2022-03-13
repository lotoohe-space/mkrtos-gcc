//
// Created by Administrator on 2022/1/30/030.
//

#include <type.h>
#include <fcntl.h>
#include <string.h>
#include "stdio.h"
#include "mkrtos/fs.h"

struct user{
    //用户id
    uint32_t user_id;
    //用户组id
    uint32_t user_group_id;
    //用户名
    const char* user_name;
    //用户密码
    const char* pwd;
    //是否再活动中
    uint8_t act;
};

//默认的用户配置文件路劲
#define USER_CFG_PATH "/usr/user.cfg"

#define DEFAULT_USER_NAME "root"
#define DEFAULT_USER_PWD ""

#define MAX_USER_NUM 6
struct user users[MAX_USER_NUM]={
        //没有读取到用户信息时的默认用户配置信息
        [0]={
                .pwd=DEFAULT_USER_PWD,
                .user_name=DEFAULT_USER_NAME,
                .user_id=0,
                .user_group_id=0
        }
};
//“root:root:0:/bin/zsh”
//读用户的配置的配置信息
void read_user_cfg(void){
    //读用户配置文件
    int32_t fd;
    int32_t res;
    char buf[128];
    FILE *fp;
    fp=fopen("/etc/passwd","r");
    if(fp==NULL) {
        printk("not find passwd file.\r\n");
        return;
    }
    fgets(buf,sizeof(buf),fp);

    char *tmp=&buf;
    char*p;
    p=strsep(&tmp,":");
    while(p!=NULL){
        printk("%s\n",p);
        p=strsep(&tmp,":");


    fclose(fp);
//    fd=open(USER_CFG_PATH,O_RDONLY,0777);
//    if(fd<0){
//        goto next;
//    }
//    res=read(fd,)
//    goto end;
    next:

    //读配置文件失败则到这里，使用默认的root用户信息
    end:

    return ;
}

/**
 * 用户进行登陆，登陆后创建第一个进程
 * @param user_name
 * @param user_pwd
 * @return
 */
int32_t user_login(const char* user_name,const char* user_pwd){
    int32_t i;
    for(i=0;i<MAX_USER_NUM;i++){
        if(users[i].act){
           continue;
        }
        if(strcmp(user_name,users[i].user_name)==0
            && strcmp(user_pwd,users[i].pwd)==0
        ){
            users[i].act=1;
            return 1;
        }
    }
    return 0;
}
/**
 * 登出后，销毁所有的进程
 * @param user_name
 * @return
 */
int32_t logout(const char* user_name){
    int32_t i;
    for(i=0;i<MAX_USER_NUM;i++){
        if(!users[i].act){
            continue;
        }
        if(strcmp(user_name,users[i].user_name)==0){
            users[i].act=0;
            return 1;
        }
    }
    return 0;
}



