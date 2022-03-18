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

__lwip_dev lwipdev;						//lwip���ƽṹ�� 
struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�

volatile PSemHandler dm9000input;					//DM9000���������ź���
volatile PMutexHandler dm9000lock;					//DM9000��д���������ź���

//DM9000���ݽ��մ�������
void lwip_dm9000_input_task(void *pdata)
{
	//�����绺�����ж�ȡ���յ������ݰ������䷢�͸�LWIP���� 
	ethernetif_input(&lwip_netif);
}

//lwip Ĭ��IP����
//lwipx:lwip���ƽṹ��ָ��
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//Ĭ��Զ��IPΪ:192.168.1.106
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=106;
	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	lwipx->mac[0]=dm9000cfg.mac_addr[0];
	lwipx->mac[1]=dm9000cfg.mac_addr[1];
	lwipx->mac[2]=dm9000cfg.mac_addr[2];
	lwipx->mac[3]=dm9000cfg.mac_addr[3];
	lwipx->mac[4]=dm9000cfg.mac_addr[4];
	lwipx->mac[5]=dm9000cfg.mac_addr[5]; 
	//Ĭ�ϱ���IPΪ:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=1;
	lwipx->ip[3]=30;
	//Ĭ����������:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//Ĭ������:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//û��DHCP	
}
extern u8 MempPoolsInit(void);
//lwip�ں˲���,�ڴ�����
//����ֵ:0,�ɹ�;
//    ����,ʧ��
u8 lwip_comm_mem_malloc(void)
{
	
	u32 ramheapsize; 
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//�õ�ram heap��С
	
	ram_heap=OSMallocEx(ramheapsize);	//������ʱ��OS���ڴ����뺯��,Ϊram_heap�����ڴ� 
	if(!ram_heap){//������ʧ�ܵ�
		return 1;
	}
	if(MempPoolsInit()==0){
		//�ڴ�����ʧ��
		return 1;
	}
	return 0;	
}

//LWIP��ʼ��(LWIP������ʱ��ʹ��)
//����ֵ:0,�ɹ�
//      1,�ڴ����
//      2,DM9000��ʼ��ʧ��
//      3,�������ʧ��.
u8 lwip_comm_init(void)
{
	uint32_t lev;
	u8 err;
	struct netif *Netif_Init_Flag;		//����netif_add()����ʱ�ķ���ֵ,�����ж������ʼ���Ƿ�ɹ�
	struct ip4_addr ipaddr;  			//ip��ַ
	struct ip4_addr netmask; 			//��������
	struct ip4_addr gw;      			//Ĭ������ 
 	//if(lwip_comm_mem_malloc())return 1;	//�ڴ�����ʧ��
 	dm9000input=SemCreateEx(0,1);			//�������ݽ����ź���,������DM9000��ʼ��֮ǰ����
 	dm9000lock=MutexCreate();			//���������ź���,��ߵ����ȼ�4	
	
	lwip_comm_mem_malloc();

	if(DM9000_Init(1)){
		return 2;			//��ʼ��DM9000AEP
	}
	tcpip_init(NULL,NULL);				//��ʼ��tcp ip�ں�,�ú�������ᴴ��tcpip_thread�ں�����
	lwip_comm_default_ip_set(&lwipdev);	//����Ĭ��IP����Ϣ
#if LWIP_DHCP		//ʹ�ö�̬IP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);

	printk("MAC Address:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
    printk("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
    printk("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
    printk("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);

#endif
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);//�������б������һ������
	if(Netif_Init_Flag != NULL) 	//������ӳɹ���,����netifΪĬ��ֵ,���Ҵ�netif����
	{
		netif_set_default(&lwip_netif); //����netifΪĬ������
		netif_set_up(&lwip_netif);		//��netif����
	}
	
#if	LWIP_DHCP
	lwip_comm_dhcp_creat();			//����DHCP����
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
	if(TaskCreate(&tcp,NULL,&osErr)<0){ 		 //��̫�����ݽ�������
		printk("�������ݽ����̳߳�ʼ������");
		return 3;
	}
	return 0;//����OK.
}   
