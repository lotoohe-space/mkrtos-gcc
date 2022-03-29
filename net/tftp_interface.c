#include "mkrtos/fs.h"
#include <fcntl.h>
#include "lwip/apps/tftp_server.h"
#include <unistd.h>

void* tftp_open(const char* fname, const char* mode, u8_t write){
	int fp;
	if((fp=sys_open(fname,write==1?(O_TRUNC|O_WRONLY|O_CREAT):O_RDONLY,0xff))<0){
		return NULL;
	}
	return (void*)fp;
}
void tftp_close(void* handle){
	sys_close((int32_t)handle);
}
int tftp_read(void* handle, void* buf, int bytes){
	int res=sys_read((uint32_t)handle,buf,bytes);
	return res;
}
int tftp_write(void* handle, struct pbuf* p){
	int br;
	int bw;
	int res;
	for(;p;){
		br=p->len;
		res = sys_write((uint32_t)handle, p->payload, br);            /* Write it to the destination file */
		if (res<=0) return -1; /* error or disk full */
		p=p->next;
	}
	return 0;
}
const struct tftp_context tftpServer={
	.open=tftp_open,
	.close=tftp_close,
	.read=tftp_read,
	.write=tftp_write
};

int32_t TftpInit(void){
	return tftp_init(&tftpServer);
}


