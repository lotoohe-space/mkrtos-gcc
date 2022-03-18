
#include "lwip/apps/lwiperf.h"

void LwiperfInit(void){
	
	ip_addr_t  perf_server_ip;  
	IP_ADDR4( &perf_server_ip, 192, 168, 1, 30 ); //IP 为 407 自己的 静态IP
	lwiperf_start_tcp_server( &perf_server_ip, 9527, NULL, NULL );
}
