//
// Created by Administrator on 2022/1/22.
//
#include <type.h>
#include <string.h>
#define FILE_NAMES_COUNT 3
#define FILE_MAX_LEN 128
static char file_names[FILE_NAMES_COUNT][FILE_MAX_LEN]={0};
static int32_t st_i=-1;

int32_t _getname(const char* file_name,uint32_t len,char** to){
    uint32_t i;
    if((st_i+1)>=FILE_NAMES_COUNT){
        return -1;
    }
    st_i++;
    for(i=0;i<len&&i<(FILE_MAX_LEN-1);i++){
        file_names[st_i][i]=file_name[i];
    }
    file_names[st_i][i]='\0';
    *to=file_names[st_i];
    return 0;
}
int32_t getname(const char* file_name, char** to){

    if((st_i+1)>=FILE_NAMES_COUNT){
        return -1;
    }
    st_i++;
    strncpy(file_names[st_i],file_name,FILE_MAX_LEN);
    *to=file_names[st_i];
    return 0;
}

void putname(const char* file_name){
    if(st_i>=0) {
        st_i--;
    }
}

char* path_name(char* file_path){
    uint32_t len;
    int32_t i;
    if (file_path == NULL) {
        return 0;
    }
    len = strlen(file_path);
    for (i = len - 1; i >= 0; i--) {
        if (file_path[i] == '\\' || file_path[i] == '/') {
            file_path[i]='\0';
            break;
        }
    }
    return &file_path[i+1];
}

