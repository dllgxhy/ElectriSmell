#ifndef __USART_H
#define __USART_H

#include <stm32f10x.h>

/****************************************************************************************
宏定义区
****************************************************************************************/
#define DEBUG_DEV       0
#define BLUE_TOOTH_DEV  1
#define OTHER_DEV       2

#define USART_DEV_CNT   3
#define USART_MAX_LEN   128

/****************************************************************************************
结构体定义区
****************************************************************************************/
typedef struct __USART_INFO
{
    USART_TypeDef*      pUart;
    unsigned int        irq;
    unsigned int        apb;

    GPIO_TypeDef*       port;
    unsigned int        tx_pin;
    unsigned int        rx_pin;

    unsigned int        rx_read_idx;
    unsigned int        rx_write_idx;
    unsigned int        tx_read_idx;
    unsigned int        tx_write_idx;

    unsigned char       rx_buf[USART_MAX_LEN];
    unsigned char       tx_buf[USART_MAX_LEN];
}USART_INFO;

/****************************************************************************************
函数声明区
****************************************************************************************/
extern unsigned int gDebugFlag ;
extern void InfoPrintf (char *fmt, ...);
extern void BluetoothPrintf(char *fmt, ...);

extern int UART_Read(int dev, unsigned char *buf, int size );
extern int UART_Write( int dev, unsigned char *buf, int size );

#define DebugPrintf(mode,fmt,...) do{\
	if(mode & gDebugFlag) \
		InfoPrintf(fmt,##__VA_ARGS__ );\
		}while(0)

#define DebugPut(mode,buff,len) do{\
	if(mode & gDebugFlag) \
		UART_Write(DEBUG_UART0,buff,len);\
		}while(0)	
extern void debug_printf (unsigned long mode,unsigned char* p_data, unsigned short len);
extern void uart_init(u8 dev, u32 bound);

#endif


