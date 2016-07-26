#include "includes.h"					//ucos 使用	  
#include "usart.h"	  
#include "debug.h"

#define CMD_BUFFER_LEN 128

/****************************************************************************************
数据定义区
****************************************************************************************/
static USART_INFO usart_list[] = {
    {USART1,USART1_IRQn,RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,GPIOA,GPIO_Pin_9 ,GPIO_Pin_10,0,0,0,0,{0},{0}},
    {USART2,USART2_IRQn,RCC_APB1Periph_USART2|RCC_APB2Periph_GPIOA,GPIOA,GPIO_Pin_2 ,GPIO_Pin_3 ,0,0,0,0,{0},{0}},
    {USART3,USART3_IRQn,RCC_APB1Periph_USART3|RCC_APB2Periph_GPIOB,GPIOB,GPIO_Pin_10,GPIO_Pin_11,0,0,0,0,{0},{0}},
};
/****************************************************************************************
静态变量定义区
****************************************************************************************/

/****************************************************************************************
获取低位ASC字符
****************************************************************************************/
static unsigned char Comm_GetAscLSB ( unsigned char uHex )
{
	uHex &= 0x0f ;

	if ( uHex > 9 )	{
		return (uHex+0X37);	
	} else {
		return (uHex+0X30);
	}
}

/****************************************************************************************
获取高位ASC字符
****************************************************************************************/
static unsigned char Comm_GetAscMSB ( unsigned char uHex )
{
	uHex = uHex >> 4 ;
	if ( uHex > 9 )		//A~F
	{
		return (uHex+0X37);	
	}
	else				//0~9
	{
		return (uHex+0X30);
	}

}
/****************************************************************************************
 串口中断服务子程序
****************************************************************************************/
static void usart_irq(u32 which)
{
	USART_INFO*       p_uart;

	if( which >= USART_DEV_CNT )
	    return;
    p_uart = &usart_list[which];
	if(USART_GetITStatus(p_uart->pUart, USART_IT_RXNE) != RESET)
	{
    	p_uart->rx_buf[p_uart->rx_write_idx] = USART_ReceiveData(p_uart->pUart);
    	p_uart->rx_write_idx ++;
    	p_uart->rx_write_idx &=(USART_MAX_LEN-1);
	}
}

/****************************************************************************************
初始化程序
****************************************************************************************/
void uart_init(u8 dev, u32 bound)
{
    //GPIO端口设置
    GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	USART_INFO*       p_uart;

	if( dev >= USART_DEV_CNT )
	    return;

	p_uart = &usart_list[dev];
    if( dev ==  0 )
        RCC_APB2PeriphClockCmd(p_uart->apb, ENABLE); //使能USART，GPIO时钟以及复用功能时钟
    else
        RCC_APB1PeriphClockCmd(p_uart->apb, ENABLE);

	//USART TX
	GPIO_InitStructure.GPIO_Pin   = p_uart->tx_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_Init(p_uart->port, &GPIO_InitStructure);
	
	//USART RX
	GPIO_InitStructure.GPIO_Pin  = p_uart->rx_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(p_uart->port, &GPIO_InitStructure);  
	
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = p_uart->irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 	 //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		 //IRQ通道使能
	NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化VIC寄存器
	
	//USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;      //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;         //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //收发模式
	
	USART_Init(p_uart->pUart,&USART_InitStructure); //初始化串口
	USART_ITConfig(p_uart->pUart, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(p_uart->pUart, ENABLE); 				   //使能串口 
}

/****************************************************************************************
从串口中读出数据
****************************************************************************************/
int UART_Read(int dev, unsigned char *buf, int size )
{
    int             i        = 0;
	USART_INFO*     p_uart   = NULL;

	if( dev >= USART_DEV_CNT )
	    return 0;

	p_uart = &usart_list[dev];
	for( i = 0 ; i < size ; i++ )
	{
	    if( p_uart->rx_write_idx == p_uart->rx_read_idx )
	        break;
	    buf[i] = p_uart->rx_buf[p_uart->rx_read_idx] ;
	    p_uart->rx_read_idx ++;
	    p_uart->rx_read_idx &= (USART_MAX_LEN-1);
	}
	return i;
}
/****************************************************************************************
向串口中写入数据
****************************************************************************************/
int UART_Write( int dev, unsigned char *buf, int size )
{
    int             i        = 0;
    unsigned int    time_out = 1000000;
	USART_INFO*     p_uart   = NULL;

	if( dev >= USART_DEV_CNT )
	    return 0;

	p_uart = &usart_list[dev];

	for( i = 0 ; i < size ; i++ )
	{
    	while(USART_GetFlagStatus(p_uart->pUart,USART_FLAG_TC)==RESET && time_out--); 
    	if( !time_out )
    	    break;
        USART_SendData(p_uart->pUart,buf[i]);
    }
    return i;
}

/****************************************************************************************
调试串口打印函数
****************************************************************************************/
void InfoPrintf (char *fmt, ...)
{
    char buffer[CMD_BUFFER_LEN];  // CMD_BUFFER_LEN长度自己定义吧 
    unsigned int len;
    va_list arg_ptr;

    va_start(arg_ptr, fmt);  
    len = vsnprintf(buffer, CMD_BUFFER_LEN, fmt, arg_ptr);
    if(buffer[len-1] == '\n' && buffer[len-2] != '\r')
    {
        if(len == CMD_BUFFER_LEN)
            len--;
        buffer[len-1] = '\r';
        buffer[len] = '\n';
        len++;
    }
    UART_Write(DEBUG_DEV, (unsigned char *)buffer,len);
    va_end(arg_ptr);
}

/****************************************************************************************
蓝牙串口打印函数
****************************************************************************************/
void BluetoothPrintf (char *fmt, ...)
{
    char buffer[CMD_BUFFER_LEN];  // CMD_BUFFER_LEN长度自己定义吧 
    unsigned int len;
    va_list arg_ptr;

    va_start(arg_ptr, fmt);  
    len = vsnprintf(buffer, CMD_BUFFER_LEN, fmt, arg_ptr);
    if(buffer[len-1] == '\n' && buffer[len-2] != '\r')
    {
        if(len == CMD_BUFFER_LEN)
            len--;
        buffer[len-1] = '\r';
        buffer[len] = '\n';
        len++;
    }
    UART_Write(BLUE_TOOTH_DEV, (unsigned char *)buffer,len);
    va_end(arg_ptr);
}

/****************************************************************************************
调试数据打印
****************************************************************************************/
void debug_printf (unsigned long mode,unsigned char* p_data, unsigned short len)
{
    unsigned char   buf[CMD_BUFFER_LEN] = {0};
    unsigned short  i        = 0;
    unsigned short  j        = 0;
    unsigned short  size     = 0;

    while( len )
    {
        if( len >= 36 )
            size = 36;
        else
            size = len;

        for( i = 0 ; i < size ; i++,j++ )
        {
            buf[i * 2 + 0] = Comm_GetAscMSB(p_data[j]);
            buf[i * 2 + 1] = Comm_GetAscLSB(p_data[j]);
        }

        len -= size;
        if( !len )
        {
            buf[i*2+0] = 0x0d;
            buf[i*2+1] = 0x0a;
            buf[i*2+2] = 0x00;
            buf[i*2+3] = 0x00;
        }
        DebugPrintf(mode,"%s",buf);
    }
}

/****************************************************************************************
 串口1中断服务程序
****************************************************************************************/
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	OSIntEnter();    
	usart_irq(DEBUG_DEV);
	OSIntExit();  											 
} 
/****************************************************************************************
 串口2中断服务程序
****************************************************************************************/
void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	OSIntEnter();    
	usart_irq(BLUE_TOOTH_DEV);
	OSIntExit();  											 
} 
/****************************************************************************************
 串口3中断服务程序
****************************************************************************************/
void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	OSIntEnter();    
	usart_irq(OTHER_DEV);
	OSIntExit();  											 
} 

