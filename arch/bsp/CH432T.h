#ifndef _CH432T_H__
#define _CH432T_H__
#include "sys.h"

#define REG_RBR_ADDR        0x00      /* 串口0接收缓冲寄存器地址 */
#define REG_THR_ADDR        0x00      /* 串口0发送保持寄存器地址 */
#define REG_IER_ADDR        0x01      /* 串口0中断使能寄存器地址 */
#define REG_IIR_ADDR        0x02      /* 串口0中断识别寄存器地址 */
#define REG_FCR_ADDR        0x02      /* 串口0FIFO控制寄存器地址 */
#define REG_LCR_ADDR        0x03      /* 串口0线路控制寄存器地址 */
#define REG_MCR_ADDR        0x04      /* 串口0MODEM控制寄存器地址 */
#define REG_LSR_ADDR        0x05      /* 串口0线路状态寄存器地址 */
#define REG_MSR_ADDR        0x06      /* 串口0MODEM状态寄存器地址 */
#define REG_SCR_ADDR        0x07      /* 串口0用户可定义寄存器地址 */
#define REG_DLL_ADDR        0x00      /* 波特率除数锁存器低8位字节地址 */
#define REG_DLM_ADDR        0x01      /* 波特率除数锁存器高8位字节地址 */

/* CH432串口1寄存器地址 */

#define REG_RBR1_ADDR       0x08      /* 串口1接收缓冲寄存器地址 */
#define REG_THR1_ADDR       0x08      /* 串口1发送保持寄存器地址 */
#define REG_IER1_ADDR       0x09      /* 串口1中断使能寄存器地址 */
#define REG_IIR1_ADDR       0x0A      /* 串口1中断识别寄存器地址 */
#define REG_FCR1_ADDR       0x0A      /* 串口1FIFO控制寄存器地址 */
#define REG_LCR1_ADDR       0x0B      /* 串口1线路控制寄存器地址 */
#define REG_MCR1_ADDR       0x0C      /* 串口1MODEM控制寄存器地址 */
#define REG_LSR1_ADDR       0x0D      /* 串口1线路状态寄存器地址 */
#define REG_MSR1_ADDR       0x0E      /* 串口1MODEM状态寄存器地址 */
#define REG_SCR1_ADDR       0x0F      /* 串口1用户可定义寄存器地址 */
#define REG_DLL1_ADDR       0x08      /* 波特率除数锁存器低8位字节地址 */
#define REG_DLM1_ADDR       0x09      /* 波特率除数锁存器高8位字节地址 */

/* IER寄存器的位 */

#define BIT_IER_RESET       0x80      /* 该位置1则软复位该串口 */
#define BIT_IER_LOWPOWER    0x40      /* 该位为1则关闭该串口的内部基准时钟 */
#define BIT_IER_SLP         0x20      /* 串口0是SLP,为1则关闭时钟震荡器 */
#define BIT_IER1_CK2X       0x20      /* 串口1是CK2X,为1则强制将外部时钟信号2倍频后作为内部基准时钟 */
#define BIT_IER_IEMODEM     0x08      /* 该位为1允许MODEM输入状态变化中断 */
#define BIT_IER_IELINES     0x04      /* 该位为1允许接收线路状态中断 */
#define BIT_IER_IETHRE      0x02      /* 该位为1允许发送保持寄存器空中断 */
#define BIT_IER_IERECV      0x01      /* 该位为1允许接收到数据中断 */

/* IIR寄存器的位 */

#define BIT_IIR_FIFOENS1    0x80
#define BIT_IIR_FIFOENS0    0x40      /* 该2位为1表示起用FIFO */

/* 中断类型：0001没有中断，0110接收线路状态中断，0100接收数据可用中断，
1100接收数据超时中断，0010THR寄存器空中断，0000MODEM输入变化中断 */
#define BIT_IIR_IID3        0x08
#define BIT_IIR_IID2        0x04
#define BIT_IIR_IID1        0x02
#define BIT_IIR_NOINT       0x01

/* FCR寄存器的位 */

/* 触发点： 00对应1个字节，01对应4个字节，10对应8个字节，11对应14个字节 */
#define BIT_FCR_RECVTG1     0x80      /* 设置FIFO的中断和自动硬件流控制的触发点 */
#define BIT_FCR_RECVTG0     0x40      /* 设置FIFO的中断和自动硬件流控制的触发点 */

#define BIT_FCR_TFIFORST    0x04      /* 该位置1则清空发送FIFO中的数据 */
#define BIT_FCR_RFIFORST    0x02      /* 该位置1则清空接收FIFO中的数据 */
#define BIT_FCR_FIFOEN      0x01      /* 该位置1则起用FIFO,为0则禁用FIFO */

/* LCR寄存器的位 */

#define BIT_LCR_DLAB        0x80      /* 为1才能存取DLL，DLM，为0才能存取RBR/THR/IER */
#define BIT_LCR_BREAKEN     0x40      /* 为1则强制产生BREAK线路间隔*/

/* 设置校验格式：当PAREN为1时，00奇校验，01偶校验，10标志位（MARK，置1)，11空白位（SPACE，清0) */
#define BIT_LCR_PARMODE1    0x20      /* 设置奇偶校验位格式 */
#define BIT_LCR_PARMODE0    0x10      /* 设置奇偶校验位格式 */

#define BIT_LCR_PAREN       0x08      /* 为1则允许发送时产生和接收校验奇偶校验位 */
#define BIT_LCR_STOPBIT     0x04      /* 为1则两个停止位,为0一个停止位 */

/* 设置字长度：00则5个数据位，01则6个数据位，10则7个数据位，11则8个数据位 */
#define BIT_LCR_WORDSZ1     0x02      /* 设置字长长度 */
#define BIT_LCR_WORDSZ0     0x01

/* MCR寄存器的位 */

#define BIT_MCR_AFE         0x20      /* 为1允许CTS和RTS硬件自动流控制 */
#define BIT_MCR_LOOP        0x10      /* 为1使能内部回路的测试模式 */
#define BIT_MCR_OUT2        0x08      /* 为1允许该串口的中断请求输出 */
#define BIT_MCR_OUT1        0x04      /* 为用户定义的MODEM控制位 */
#define BIT_MCR_RTS         0x02      /* 该位为1则RTS引脚输出有效 */
#define BIT_MCR_DTR         0x01      /* 该位为1则DTR引脚输出有效 */

/* LSR寄存器的位 */

#define BIT_LSR_RFIFOERR    0x80      /* 为1表示在接收FIFO中存在至少一个错误 */
#define BIT_LSR_TEMT        0x40      /* 为1表示THR和TSR全空 */
#define BIT_LSR_THRE        0x20      /* 为1表示THR空*/
#define BIT_LSR_BREAKINT    0x10      /* 该位为1表示检测到BREAK线路间隔 */
#define BIT_LSR_FRAMEERR    0x08      /* 该位为1表示读取数据帧错误 */
#define BIT_LSR_PARERR      0x04      /* 该位为1表示奇偶校验错误 */
#define BIT_LSR_OVERR       0x02      /* 为1表示接收FIFO缓冲区溢出 */
#define BIT_LSR_DATARDY     0x01      /* 该位为1表示接收FIFO中有接收到的数据 */

/* MSR寄存器的位 */

#define BIT_MSR_DCD         0x80      /* 该位为1表示DCD引脚有效 */
#define BIT_MSR_RI          0x40      /* 该位为1表示RI引脚有效 */
#define BIT_MSR_DSR         0x20      /* 该位为1表示DSR引脚有效 */
#define BIT_MSR_CTS         0x10      /* 该位为1表示CTS引脚有效 */
#define BIT_MSR_DDCD        0x08      /* 该位为1表示DCD引脚输入状态发生变化过 */
#define BIT_MSR_TERI        0x04      /* 该位为1表示RI引脚输入状态发生变化过 */
#define BIT_MSR_DDSR        0x02      /* 该位为1表示DSR引脚输入状态发生变化过 */
#define BIT_MSR_DCTS        0x01      /* 该位为1表示CTS引脚输入状态发生变化过 */

/* 中断状态码 */

#define INT_NOINT           0x01      /* 没有中断 */
#define INT_THR_EMPTY       0x02      /* THR空中断 */
#define INT_RCV_OVERTIME    0x0C      /* 接收超时中断 */
#define INT_RCV_SUCCESS     0x04      /* 接收数据可用中断 */
#define INT_RCV_LINES       0x06      /* 接收线路状态中断 */
#define INT_MODEM_CHANGE    0x00      /* MODEM输入变化中断 */
/**********************************************************************
        定义CH432串口0的寄存器地址
***********************************************************************/

#define CH432_RBR_PORT     REG_RBR_ADDR     /* 假定CH432接收缓冲寄存器0的I/O地址 */
#define CH432_THR_PORT     REG_THR_ADDR     /* 假定CH432发送保持寄存器0的I/O地址 */
#define CH432_IER_PORT     REG_IER_ADDR     /* 假定CH432中断使能寄存器0的I/O地址 */
#define CH432_IIR_PORT     REG_IIR_ADDR     /* 假定CH432中断识别寄存器0的I/O地址 */
#define CH432_FCR_PORT     REG_FCR_ADDR     /* 假定CH432FIFO控制寄存器0的I/O地址 */
#define CH432_LCR_PORT     REG_LCR_ADDR     /* 假定CH432线路控制寄存器0的I/O地址 */
#define CH432_MCR_PORT     REG_MCR_ADDR     /* 假定CH432MODEM控制寄存器0的I/O地址 */
#define CH432_LSR_PORT     REG_LSR_ADDR     /* 假定CH432线路状态寄存器0的I/O地址 */
#define CH432_MSR_PORT     REG_MSR_ADDR     /* 假定CH432MODEM状态寄存器0的I/O地址 */
#define CH432_SCR_PORT     REG_SCR_ADDR     /* 假定CH432用户可定义寄存器0的I/O地址 */
#define CH432_DLL_PORT     REG_DLL_ADDR
#define CH432_DLM_PORT     REG_DLM_ADDR

/**************************************************************************
        定义CH432串口1的寄存器地址
**************************************************************************/

#define CH432_RBR1_PORT    REG_RBR1_ADDR    /* 假定CH432接收缓冲寄存器1的I/O地址 */
#define CH432_THR1_PORT    REG_THR1_ADDR    /* 假定CH432发送保持寄存器1的I/O地址 */
#define CH432_IER1_PORT    REG_IER1_ADDR    /* 假定CH432中断使能寄存器1的I/O地址 */
#define CH432_IIR1_PORT    REG_IIR1_ADDR    /* 假定CH432中断识别寄存器1的I/O地址 */
#define CH432_FCR1_PORT    REG_FCR1_ADDR    /* 假定CH432FIFO控制寄存器1的I/O地址 */
#define CH432_LCR1_PORT    REG_LCR1_ADDR    /* 假定CH432线路控制寄存器1的I/O地址 */
#define CH432_MCR1_PORT    REG_MCR1_ADDR    /* 假定CH432MODEM控制寄存器1的I/O地址 */
#define CH432_LSR1_PORT    REG_LSR1_ADDR    /* 假定CH432线路状态寄存器1的I/O地址 */
#define CH432_MSR1_PORT    REG_MSR1_ADDR    /* 假定CH432MODEM状态寄存器1的I/O地址 */
#define CH432_SCR1_PORT    REG_SCR1_ADDR    /* 假定CH432用户可定义寄存器1的I/O地址 */
#define CH432_DLL1_PORT    REG_DLL1_ADDR
#define CH432_DLM1_PORT    REG_DLM1_ADDR



#define CH432_IIR_FIFOS_ENABLED 0xC0  /* 起用FIFO */


typedef void (*CH432T_recv_data_call_back)(u8 port,u8 * data,u16 len);
extern CH432T_recv_data_call_back CH432T_recv_0_data_fun;
extern CH432T_recv_data_call_back CH432T_recv_1_data_fun;

uint8_t Ch432_SPI_ReadWriteByte(uint8_t data);
void Ch432_SPI_SpeedLow(void);
void Ch432_SPI_SpeedHigh(void);
void WriteCH432Data( uint8_t mAddr, uint8_t mData );
uint8_t ReadCH432Data( uint8_t mAddr );
int Ch432_SPI_Init(void);
void UART0_SendByte( uint8_t dat );
void ch432t_reset(int inx);
void ch432t_init_intr(int inx);
void UART1_SendByte( uint8_t dat ) ;
uint8_t UART1_RcvByte(void) ;

void  CH432Seril0Send( u8 *Data, u8 Num );    /* 禁用FIFO,CH432串口0发送多字节子程序 */
void  CH432Seril1Send( u8 *Data, u8 Num );    /* 禁用FIFO,CH432串口1发送多字节子程序 */

void ch432t_set_baud(int32_t inx,int32_t baud);
void ch4324_set_par(int32_t inx,int32_t csize,int32_t cstopb,int32_t par);
void ch432t_flow_ctrl(int32_t inx,int32_t status);

#endif
