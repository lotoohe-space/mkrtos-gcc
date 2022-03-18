#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "netif/ethernetif.h" 
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h" 

#include "sys.h"
#include "dm9000.h"
#include "unistd.h"
#include "mkrtos/mem.h"
#include "net_init.h"

__lwip_dev lwipdev;						//lwip控制结构体 
struct netif lwip_netif;				//定义一个全局的网络接口

volatile PSemHandler dm9000input;					//DM9000接收数据信号量
volatile PMutexHandler dm9000lock;					//DM9000读写互锁控制信号量

//DM9000数据接收处理任务
void lwip_dm9000_input_task(void *pdata)
{
	//从网络缓冲区中读取接收到的数据包并将其发送给LWIP处理 
	ethernetif_input(&lwip_netif);
}

//lwip 默认IP设置
//lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//默认远端IP为:192.168.1.106
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=106;
	//MAC地址设置(高三字节固定为:2.0.0,低三字节用STM32唯一ID)
	lwipx->mac[0]=dm9000cfg.mac_addr[0];
	lwipx->mac[1]=dm9000cfg.mac_addr[1];
	lwipx->mac[2]=dm9000cfg.mac_addr[2];
	lwipx->mac[3]=dm9000cfg.mac_addr[3];
	lwipx->mac[4]=dm9000cfg.mac_addr[4];
	lwipx->mac[5]=dm9000cfg.mac_addr[5]; 
	//默认本地IP为:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=1;
	lwipx->ip[3]=30;
	//默认子网掩码:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//默认网关:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//没有DHCP	
}
extern u8 MempPoolsInit(void);
//lwip内核部分,内存申请
//返回值:0,成功;
//    其他,失败
u8 lwip_comm_mem_malloc(void)
{
	
	u32 ramheapsize; 
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//得到ram heap大小
	
	ram_heap=OSMallocEx(ramheapsize);	//这里暂时用OS的内存申请函数,为ram_heap申请内存 
	if(!ram_heap){//有申请失败的
		return 1;
	}
	if(MempPoolsInit()==0){
		//内存申请失败
		return 1;
	}
	return 0;	
}

//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,DM9000初始化失败
//      3,网卡添加失败.
u8 lwip_comm_init(void)
{
	uint32_t lev;
	u8 err;
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip4_addr ipaddr;  			//ip地址
	struct ip4_addr netmask; 			//子网掩码
	struct ip4_addr gw;      			//默认网关 
 	//if(lwip_comm_mem_malloc())return 1;	//内存申请失败
 	dm9000input=SemCreateEx(0,1);			//创建数据接收信号量,必须在DM9000初始化之前创建
 	dm9000lock=MutexCreate();			//创建互斥信号量,提高到优先级4	
	
	lwip_comm_mem_malloc();

	if(DM9000_Init(1)){
		return 2;			//初始化DM9000AEP
	}
	tcpip_init(NULL,NULL);				//初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务
	lwip_comm_default_ip_set(&lwipdev);	//设置默认IP等信息
#if LWIP_DHCP		//使用动态IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);

	printk("MAC Address:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
    printk("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
    printk("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
    printk("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);

#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//向网卡列表中添加一个网口
	if(Netif_Init_Flag != NULL) 	//网口添加成功后,设置netif为默认值,并且打开netif网口
	{
		netif_set_default(&lwip_netif); //设置netif为默认网口
		netif_set_up(&lwip_netif);		//打开netif网口
	}
	
#if	LWIP_DHCP
	lwip_comm_dhcp_creat();			//创建DHCP任务
#endif	
	
	ip_addr_t dns_addr;
	IP_ADDR4(&dns_addr,223,5,5,5);
	dns_setserver(0, &dns_addr);
	
	OSError osErr;
	TaskCreatePar tcp;

	tcp.taskFun=lwip_dm9000_input_task;
	tcp.arg0=(void*)0;
	tcp.arg1=0;
	tcp.prio=6;
//	tcp.threadMode=1;
	tcp.userStackSize=0;
	tcp.kernelStackSize=512;
	tcp.taskName="NetRecvThread";
	if(TaskCreate(&tcp,NULL,&osErr)<0){ 		 //以太网数据接收任务
		printk("网口数据接收线程初始化错误");
		return 3;
	}
	return 0;//操作OK.
}   
