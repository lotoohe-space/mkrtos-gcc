
#include "lwip/apps/lwiperf.h"

void LwiperfInit(void){
	
	ip_addr_t  perf_server_ip;  
	IP_ADDR4( &perf_server_ip, 192, 168, 1, 30 ); //IP Ϊ 407 �Լ��� ��̬IP
	lwiperf_start_tcp_server( &perf_server_ip, 9527, NULL, NULL );
}
