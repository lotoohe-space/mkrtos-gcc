//
// Created by Administrator on 2022/1/30/030.
//

#include <type.h>
#include <fcntl.h>
#include <string.h>
#include "stdio.h"
#include "mkrtos/fs.h"
#include "stdlib.h"
#include "paths.h"


#define USER_NAME_MAX_LEN 32
#define USER_PWD_MAX_LEN 32
#define USER_EXEC_PATH 32
#define USER_PATH 32
struct user{
    //用户id
    uint32_t user_id;
    //用户组id
    uint32_t user_group_id;
    //用户名
    char user_name[USER_NAME_MAX_LEN];
    //用户密码
    char pwd[USER_PWD_MAX_LEN];
    char exec_path[USER_EXEC_PATH];
    char user_path[USER_PATH];
    //是否再活动中
    uint8_t act;
    //是否有效
    uint8_t vaild;
};

//默认的用户配置文件路劲
#define MAX_USER_NUM 6
struct user users[MAX_USER_NUM]={
0
};

//“account:password:UID:GID:GECOS:directory:shell”
//读用户的配置的配置信息
void read_user_cfg(void){
    int read_line=0;
    //读用户配置文件
    char buf[128];
    FILE *fp;
    fp=fopen(_PATH_PASSWD,"r");
    if(fp==NULL) {
        fatalk("not find passwd file.\r\n");
        return;
    }
    while(fgets(buf,sizeof(buf),fp)) {
        int inx = 0;
        char *tmp = &buf;
        char *p;
        p = strsep(&tmp, ":");
        while (p != NULL) {
            printk("%s\n", p);
            switch (inx) {
                case 0:
                    strncpy(users[read_line].user_name, p, USER_NAME_MAX_LEN);
                    break;
                case 1:
                    strncpy(users[read_line].pwd, p, USER_NAME_MAX_LEN);
                    break;
                case 2:
                    users[read_line].user_id = atoi(p);
                    break;
                case 3:
                    users[read_line].user_group_id = atoi(p);
                    break;
                case 5:
                    strncpy(users[read_line].user_path, p, USER_PATH);
                    break;
                case 6:
                    strncpy(users[read_line].exec_path, p, USER_NAME_MAX_LEN);
                    break;
            }
            inx++;
            p = strsep(&tmp, ":");
        }
        if(inx==7){
            users[read_line].vaild=TRUE;
            read_line++;
        }else{
            printk("user file exist a error.\r\n");
        }
    }
    fclose(fp);

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



