#include "dm9000.h"
#include "delay.h"
#include "net_init.h"
#include "sys.h"
#include <mkrtos/knl_sem.h>
#include <mkrtos/knl_mutex.h>
#include "arch/isr.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//DM9000驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/3/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 

struct dm9000_config dm9000cfg;				//DM9000配置结构体

extern volatile struct sem_hdl *dm9000input;				//DM9000接收数据信号量
extern volatile struct mutex_hdl* dm9000lock;				//DM9000读写互锁控制信号量
void EXTI15_10_IRQHandler(void);
//初始化DM9000
//mode:0,仅仅读DM9000的ID,看能否读到ID.
//     1,完全初始化,正常应用时使用
//返回值:
//0,初始化成功
//1，DM9000A ID读取错误
u8 DM9000_Init(u8 mode)
{
	u32 temp; 
	//先初始化和DM9000连接的IO和FSMC
	RCC->AHBENR|=1<<8;     	 				//使能FSMC时钟	 
	RCC->APB2ENR|=1<<5;     				//使能PORTD时钟
	RCC->APB2ENR|=1<<6;     				//使能PORTE时钟
	RCC->APB2ENR|=1<<7;     				//使能PORTF时钟
 	RCC->APB2ENR|=1<<8;						//使能PORTG时钟	 
	
	//PORTD复用推挽输出 	
	GPIOD->CRH=(GPIOD->CRH&0X00FFF000)|0XBB000BBB;
	GPIOD->CRL=(GPIOD->CRH&0XFF00FF00)|0X00BB00BB;	 
	//PORTE复用推挽输出 	
	GPIOE->CRH=(GPIOE->CRH&0X00000000)|0XBBBBBBBB;
	GPIOE->CRL=(GPIOE->CRL&0X0FFFFFFF)|0XB0000000;   
	//PORTF复用推挽输出 	PF13-->FSMC_A7 PF11  DM9000中断输入,上拉
	GPIOF->CRH=(GPIOF->CRH&0XFF0F0FFF)|0X00B08000; 	
	//PORTG复用推挽输出 PG9->NE2 PG8 推挽输出--> DM9000_RST
	GPIOG->CRH=(GPIOG->CRH&0XFFFFFF00)|0X000000B3; 		
	
	GPIOF->ODR|=1<<11;						//PF11 上拉
	//DM9000_IO_Init();
	Ex_NVIC_Config(GPIO_F,11,FTIR);			//下降沿触发 
	MY_NVIC_Init(0,1,EXTI15_10_IRQn,2);		//抢占2，子优先级3，组2
	
	//注册中断
	RegIsrFunc(EXTI15_10_IRQHandler,41,0);
	
	//寄存器清零
	//bank1有NE1~4,每一个有一个BCR+TCR，所以总共八个寄存器。
	//这里我们使用NE2 ，也就对应BTCR[2],[3]。				    
	FSMC_Bank1->BTCR[2]=0X00000000;
	FSMC_Bank1->BTCR[3]=0X00000000;
	FSMC_Bank1E->BWTR[2]=0X00000000;
	//操作BCR寄存器	使用异步模式,模式A(读写共用一个时序寄存器)
	//BTCR[偶数]:BCR寄存器;BTCR[奇数]:BTR寄存器
	FSMC_Bank1->BTCR[2]|=1<<12;				//存储器写使能
	FSMC_Bank1->BTCR[2]|=1<<4; 				//存储器数据宽度为16bit 	    
	//操作BTR寄存器								    
	FSMC_Bank1->BTCR[3]|=3<<8; 				//数据保持时间（DATAST）为3个HCLK 4/72M=55ns(对DM9000芯片)	 	 
	FSMC_Bank1->BTCR[3]|=0<<4; 				//地址保持时间（ADDHLD）未用到	  	 
	FSMC_Bank1->BTCR[3]|=1<<0;				 //地址建立时间（ADDSET）为2个HCLK 2/72M=27ns	 	 
	//闪存写时序寄存器  
	FSMC_Bank1E->BWTR[2]=0x0FFFFFFF;		//默认值
	//使能BANK1区域2
	FSMC_Bank1->BTCR[2]|=1<<0; 
	
	temp=*(vu32*)(0x1FFFF7E8);				//获取STM32的唯一ID的前24位作为MAC地址后三字节
	dm9000cfg.mode=DM9000_AUTO;	
 	dm9000cfg.queue_packet_len=0;
	//DM9000的SRAM的发送和接收指针自动返回到开始地址，并且开启接收中断
	dm9000cfg.imr_all = IMR_PAR|IMR_PRI; 
	//初始化MAC地址
	dm9000cfg.mac_addr[0]=2;
	dm9000cfg.mac_addr[1]=0;
	dm9000cfg.mac_addr[2]=0;
	dm9000cfg.mac_addr[3]=(temp>>16)&0XFF;	//低三字节用STM32的唯一ID
	dm9000cfg.mac_addr[4]=(temp>>8)&0XFFF;
	dm9000cfg.mac_addr[5]=temp&0XFF;
	//初始化组播地址
	dm9000cfg.multicase_addr[0]=0Xff;
	dm9000cfg.multicase_addr[1]=0Xff;
	dm9000cfg.multicase_addr[2]=0Xff;
	dm9000cfg.multicase_addr[3]=0Xff;
	dm9000cfg.multicase_addr[4]=0Xff;
	dm9000cfg.multicase_addr[5]=0Xff;
	dm9000cfg.multicase_addr[6]=0Xff;
	dm9000cfg.multicase_addr[7]=0Xff; 
	
	DM9000_Reset();							//复位DM9000
	delay_ms(150);
	temp=DM9000_Get_DeiviceID();			//获取DM9000ID
	//printf("DM9000 ID:%#x\r\n",temp);
	if(temp!=DM9000_ID) return 1; 			//读取ID错误
	if(mode==0)return 0;					//仅仅读ID,任务完成
	DM9000_Set_PHYMode(dm9000cfg.mode);		//设置PHY工作模式
	
	DM9000_WriteReg(DM9000_NCR,0X00);
	DM9000_WriteReg(DM9000_TCR,0X00);		//发送控制寄存器清零
	DM9000_WriteReg(DM9000_BPTR,0X3F);	
	DM9000_WriteReg(DM9000_FCTR,0X38);
	DM9000_WriteReg(DM9000_FCR,0X00);
	DM9000_WriteReg(DM9000_SMCR,0X00);		//特殊模式
	DM9000_WriteReg(DM9000_NSR,NSR_WAKEST|NSR_TX2END|NSR_TX1END);//清除发送状态
	DM9000_WriteReg(DM9000_ISR,0X0F);		//清除中断状态
	DM9000_WriteReg(DM9000_TCR2,0X80);		//切换LED到mode1 	
	//设置MAC地址和组播地址
	DM9000_Set_MACAddress(dm9000cfg.mac_addr);		//设置MAC地址
	DM9000_Set_Multicast(dm9000cfg.multicase_addr);	//设置组播地址
	DM9000_WriteReg(DM9000_RCR,RCR_DIS_LONG|RCR_DIS_CRC|RCR_RXEN);
	DM9000_WriteReg(DM9000_IMR,IMR_PAR); 
//	temp=DM9000_Get_SpeedAndDuplex();		//获取DM9000的连接速度和双工状态
//	if(temp!=0XFF)							//连接成功，通过串口显示连接速度和双工状态
//	{
//		printf("DM9000 Speed:%dMbps,Duplex:%s duplex mode\r\n",(temp&0x02)?10:100,(temp&0x01)?"Full":"Half");
//	}
	//else printf("DM9000 Establish Link Failed!\r\n");
	DM9000_WriteReg(DM9000_IMR,dm9000cfg.imr_all);	//设置中断
	return 0;		
} 

//读取DM9000指定寄存器的值
//reg:寄存器地址
//返回值：DM9000指定寄存器的值
u16 DM9000_ReadReg(u16 reg)
{
	DM9000->REG=reg;
	return DM9000->DATA; 
}
//向DM9000指定寄存器中写入指定值
//reg:要写入的寄存器
//data:要写入的值
void DM9000_WriteReg(u16 reg,u16 data)
{
	DM9000->REG=reg;
	DM9000->DATA=data;
}
//读取DM9000的PHY的指定寄存器
//reg:要读的PHY寄存器
//返回值:读取到的PHY寄存器值
u16 DM9000_PHY_ReadReg(u16 reg)
{
	u16 temp;
	DM9000_WriteReg(DM9000_EPAR,DM9000_PHY|reg);
	DM9000_WriteReg(DM9000_EPCR,0X0C);				//选中PHY，发送读命令
	delay_ms(10);
	DM9000_WriteReg(DM9000_EPCR,0X00);				//清除读命令
	temp=(DM9000_ReadReg(DM9000_EPDRH)<<8)|(DM9000_ReadReg(DM9000_EPDRL));
	return temp;
}
//向DM9000的PHY寄存器写入指定值
//reg:PHY寄存器
//data:要写入的值
void DM9000_PHY_WriteReg(u16 reg,u16 data)
{
	DM9000_WriteReg(DM9000_EPAR,DM9000_PHY|reg);
	DM9000_WriteReg(DM9000_EPDRL,(data&0xff));		//写入低字节
	DM9000_WriteReg(DM9000_EPDRH,((data>>8)&0xff));	//写入高字节
	DM9000_WriteReg(DM9000_EPCR,0X0A);				//选中PHY,发送写命令
	delay_ms(50);
	DM9000_WriteReg(DM9000_EPCR,0X00);				//清除写命令	
} 
//获取DM9000的芯片ID
//返回值：DM9000的芯片ID值
u32 DM9000_Get_DeiviceID(void)
{
	u32 value;
	value =DM9000_ReadReg(DM9000_VIDL);
	value|=DM9000_ReadReg(DM9000_VIDH) << 8;
	value|=DM9000_ReadReg(DM9000_PIDL) << 16;
	value|=DM9000_ReadReg(DM9000_PIDH) << 24;
	return value;
}
//获取DM9000的连接速度和双工模式
//返回值：	0,100M半双工
//			1,100M全双工
//			2,10M半双工
//			3,10M全双工
//			0XFF,连接失败！
u8 DM9000_Get_SpeedAndDuplex(void)
{
	u8 temp;
	u8 i=0;	
	if(dm9000cfg.mode==DM9000_AUTO)					//如果开启了自动协商模式一定要等待协商完成
	{
		while(!(DM9000_PHY_ReadReg(0X01)&0X0020))	//等待自动协商完成
		{
			delay_ms(100);	
 			i++;
			if(i>100)return 0XFF;					//自动协商失败
		}	
	}else											//自定义模式,一定要等待连接成功
	{
		while(!(DM9000_ReadReg(DM9000_NSR)&0X40))	//等待连接成功
		{
			delay_ms(100);					
			i++;
			if(i>100)return 0XFF;					//连接失败			
		}
	}
	temp =((DM9000_ReadReg(DM9000_NSR)>>6)&0X02);	//获取DM9000的连接速度
	temp|=((DM9000_ReadReg(DM9000_NCR)>>3)&0X01);	//获取DM9000的双工状态
	return temp;
}

//设置DM900的PHY工作模式
//mode:PHY模式
void DM9000_Set_PHYMode(u8 mode)
{
	u16 BMCR_Value,ANAR_Value;	
	switch(mode)
	{
		case DM9000_10MHD:		//10M半双工
			BMCR_Value=0X0000;
			ANAR_Value=0X21;
			break;
		case DM9000_10MFD:		//10M全双工
			BMCR_Value=0X0100;
			ANAR_Value=0X41;
			break;
		case DM9000_100MHD:		//100M半双工
			BMCR_Value=0X2000;
			ANAR_Value=0X81;
			break;
		case DM9000_100MFD:		//100M全双工
			BMCR_Value=0X2100;
			ANAR_Value=0X101;
			break;
		case DM9000_AUTO:		//自动协商模式
			BMCR_Value=0X1000;
			ANAR_Value=0X01E1;
			break;		
	}
	DM9000_PHY_WriteReg(DM9000_PHY_BMCR,BMCR_Value);
	DM9000_PHY_WriteReg(DM9000_PHY_ANAR,ANAR_Value);
 	DM9000_WriteReg(DM9000_GPR,0X00);	//使能PHY
}
//设置DM9000的MAC地址
//macaddr:指向MAC地址
void DM9000_Set_MACAddress(u8 *macaddr)
{
	u8 i;
	for(i=0;i<6;i++)
	{
		DM9000_WriteReg(DM9000_PAR+i,macaddr[i]);
	}
}
//设置DM9000的组播地址
//multicastaddr:指向多播地址
void DM9000_Set_Multicast(u8 *multicastaddr)
{
	u8 i;
	for(i=0;i<8;i++)
	{
		DM9000_WriteReg(DM9000_MAR+i,multicastaddr[i]);
	}
}  
//复位DM9000
void DM9000_Reset(void)
{
	//复位DM9000,复位步骤参考<DM9000 Application Notes V1.22>手册29页
	DM9000_RST=0;								//DM9000硬件复位
	delay_ms(10);
	DM9000_RST=1; 								//DM9000硬件复位结束
	delay_ms(100);								//一定要有这个延时，让DM9000准备就绪！
 	DM9000_WriteReg(DM9000_GPCR,0x01);			//第一步:设置GPCR寄存器(0X1E)的bit0为1 
	DM9000_WriteReg(DM9000_GPR,0);				//第二步:设置GPR寄存器(0X1F)的bit1为0，DM9000内部的PHY上电
 	DM9000_WriteReg(DM9000_NCR,(0x02|NCR_RST));	//第三步:软件复位DM9000 
	do 
	{
		delay_ms(25); 	
	}while(DM9000_ReadReg(DM9000_NCR)&1);		//等待DM9000软复位完成
	DM9000_WriteReg(DM9000_NCR,0);
	DM9000_WriteReg(DM9000_NCR,(0x02|NCR_RST));	//DM9000第二次软复位
	do 
	{
		delay_ms(25);	
	}while (DM9000_ReadReg(DM9000_NCR)&1);
}  

//通过DM9000发送数据包
//p:pbuf结构体指针
void DM9000_SendPacket(struct pbuf *p)
{
	struct pbuf *q;
	u16 pbuf_index = 0;
	u8 word[2], word_index = 0;
//	u8 err;	
 	//printf("send len:%d\r\n",p->tot_len);
	mutex_lock(dm9000lock,0xffffffff); 			//请求互斥信号量,锁定DM9000
	DM9000_WriteReg(DM9000_IMR,IMR_PAR);		//关闭网卡中断 
	DM9000->REG=DM9000_MWCMD;					//发送此命令后就可以将要发送的数据搬到DM9000 TX SRAM中	
	q=p;
	//向DM9000的TX SRAM中写入数据，一次写入两个字节数据
	//当要发送的数据长度为奇数的时候，我们需要将最后一个字节单独写入DM9000的TX SRAM中
 	while(q)
	{
		if (pbuf_index < q->len)
		{
			word[word_index++] = ((u8_t*)q->payload)[pbuf_index++];
			if (word_index == 2)
			{
				DM9000->DATA=((u16)word[1]<<8)|word[0];
				word_index = 0;
			}
		}else
		{
			q=q->next;
			pbuf_index = 0;
		}
	}
	//还有一个字节未写入TX SRAM
	if(word_index==1)DM9000->DATA=word[0];
	//向DM9000写入发送长度
	DM9000_WriteReg(DM9000_TXPLL,p->tot_len&0XFF);
	DM9000_WriteReg(DM9000_TXPLH,(p->tot_len>>8)&0XFF);		//设置要发送数据的数据长度
	DM9000_WriteReg(DM9000_TCR,0X01);						//启动发送 
	while((DM9000_ReadReg(DM9000_ISR)&0X02)==0);			//等待发送完成 
	DM9000_WriteReg(DM9000_ISR,0X02);						//清除发送完成中断 
 	DM9000_WriteReg(DM9000_IMR,dm9000cfg.imr_all);			//DM9000网卡接收中断使能
	//DM9000_WriteReg(DM9000_IMR,IMR_POI|IMR_PTI|IMR_PRI);			//DM9000网卡接收中断使能
	mutex_unlock(dm9000lock);								//发送互斥信号量,解锁DM9000
}
//DM9000接收数据包
//接收到的数据包存放在DM9000的RX FIFO中，地址为0X0C00~0X3FFF
//接收到的数据包的前四个字节并不是真实的数据，而是有特定含义的
//byte1:表明是否接收到数据，为0x00或者0X01，如果两个都不是的话一定要软件复位DM9000
//		0x01，接收到数据
//		0x00，未接收到数据
//byte2:第二个字节表示一些状态信息，和DM9000的RSR(0X06)寄存器一致的
//byte3:本帧数据长度的低字节
//byte4:本帧数据长度的高字节
//返回值：pbuf格式的接收到的数据包
struct pbuf *DM9000_Receive_Packet(void)
{
	struct pbuf* p;
	struct pbuf* q;
    u32 rxbyte;
	vu16 rx_status, rx_length;
    u16* data;
	u16 dummy; 
	int len;
//	u8 err;

	p=NULL; 
	mutex_lock(dm9000lock,0xffffffff);				//请求互斥信号量,锁定DM9000
__error_retry:	
	DM9000_ReadReg(DM9000_MRCMDX);					//假读
	rxbyte=(u8)DM9000->DATA;						//进行第二次读取 
	if(rxbyte)										//接收到数据
	{
		if(rxbyte>1)								//rxbyte大于1，接收到的数据错误,挂了		
		{
            printk("dm9000 rx: rx error, stop device\r\n");
			DM9000_WriteReg(DM9000_RCR,0x00);
			DM9000_WriteReg(DM9000_ISR,0x80);		
			mutex_unlock(dm9000lock);							//发送互斥信号量,解锁DM9000
			return (struct pbuf*)p;
		}
		DM9000->REG=DM9000_MRCMD;
		rx_status=DM9000->DATA;
        rx_length=DM9000->DATA;  
		//if(rx_length>512)printf("rxlen:%d\r\n",rx_length);
        p=pbuf_alloc(PBUF_RAW,rx_length,PBUF_POOL);	//pbufs内存池分配pbuf
		if(p!=NULL)									//内存申请成功
        {
            for(q=p;q!=NULL;q=q->next)
            {
                data=(u16*)q->payload;
                len=q->len;
                while(len>0)
                {
										*data=DM9000->DATA;
                    data++;
                    len-= 2;
                }
            }
        }else										//内存申请失败
		{
			printk("pbuf内存申请失败\r\n");
			data=&dummy;
			len=rx_length;
			while(len)
			{
				*data=DM9000->DATA;
				len-=2;
			}
        }	
		//根据rx_status判断接收数据是否出现如下错误：FIFO溢出、CRC错误
		//对齐错误、物理层错误，如果有任何一个出现的话丢弃该数据帧，
		//当rx_length小于64或者大于最大数据长度的时候也丢弃该数据帧
		if((rx_status&0XBF00) || (rx_length < 0X40) || (rx_length > DM9000_PKT_MAX))
		{
			printk("rx_status\r\n");
			if (rx_status & 0x100)printk("rx fifo error\r\n");
            if (rx_status & 0x200)printk("rx crc error\r\n");
            if (rx_status & 0x8000)printk("rx length error\r\n");
            if (rx_length>DM9000_PKT_MAX)
			{
				printk("rx length too big\r\n");
				DM9000_WriteReg(DM9000_NCR, NCR_RST); 	//复位DM9000
				delay_ms(5);
			}
			if(p!=NULL)pbuf_free((struct pbuf*)p);		//释放内存
			p=NULL;
			goto __error_retry;
		}
	}else
    {
        DM9000_WriteReg(DM9000_ISR,ISR_PTS);			//清除所有中断标志位
        dm9000cfg.imr_all=IMR_PAR|IMR_PRI;				//重新接收中断 
        DM9000_WriteReg(DM9000_IMR, dm9000cfg.imr_all);
    } 
	mutex_unlock(dm9000lock);							//发送互斥信号量,解锁DM9000
	return (struct pbuf*)p; 
}
//中断处理函数
void DMA9000_ISRHandler(void)
{
	u16 int_status;
	u16 last_io; 
	last_io = DM9000->REG;
	int_status=DM9000_ReadReg(DM9000_ISR); 
	DM9000_WriteReg(DM9000_ISR,int_status);				//清除中断标志位，DM9000的ISR寄存器的bit0~bit5写1清零
	//if(int_status & ISR_ROS)printf("overflow \r\n");
  //  if(int_status & ISR_ROOS)printf("overflow counter overflow \r\n");	
	if(int_status & ISR_PRS)		//接收中断
	{  
 		sem_post(dm9000input);		//处理接收到数据帧
	} 
	if(int_status&ISR_PTS)			//发送中断
	{ 
									//接收中断处理,这里没用到
	}
	DM9000->REG=last_io;	
}
void EXTI15_10_IRQHandler(void)
{		
 	if(EXTI_GetITStatus(EXTI_Line11)!=RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line11);
		while(DM9000_INT==0)
		{
			DMA9000_ISRHandler();
		}		
	}
		  
} 


































