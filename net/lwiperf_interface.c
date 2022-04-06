
#include "lwip/apps/lwiperf.h"

void lwiperf_init(void){
	
	ip_addr_t  perf_server_ip;  
	IP_ADDR4( &perf_server_ip, 192, 168, 3, 120 ); //IP 为 407 自己的 静态IP
	lwiperf_start_tcp_server( &perf_server_ip, 9527, NULL, NULL );
}
