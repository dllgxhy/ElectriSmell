#include "includes.h"					//ucos ʹ��	  
#include "usart.h"	  
#include "debug.h"

#define CMD_BUFFER_LEN 128

/****************************************************************************************
���ݶ�����
****************************************************************************************/
static USART_INFO usart_list[] = {
    {USART1,USART1_IRQn,RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,GPIOA,GPIO_Pin_9 ,GPIO_Pin_10,0,0,0,0,{0},{0}},
    {USART2,USART2_IRQn,RCC_APB1Periph_USART2|RCC_APB2Periph_GPIOA,GPIOA,GPIO_Pin_2 ,GPIO_Pin_3 ,0,0,0,0,{0},{0}},
    {USART3,USART3_IRQn,RCC_APB1Periph_USART3|RCC_APB2Periph_GPIOB,GPIOB,GPIO_Pin_10,GPIO_Pin_11,0,0,0,0,{0},{0}},
};
/****************************************************************************************
��̬����������
****************************************************************************************/

/****************************************************************************************
��ȡ��λASC�ַ�
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
��ȡ��λASC�ַ�
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
 �����жϷ����ӳ���
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
��ʼ������
****************************************************************************************/
void uart_init(u8 dev, u32 bound)
{
    //GPIO�˿�����
    GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;
	USART_INFO*       p_uart;

	if( dev >= USART_DEV_CNT )
	    return;

	p_uart = &usart_list[dev];
    if( dev ==  0 )
        RCC_APB2PeriphClockCmd(p_uart->apb, ENABLE); //ʹ��USART��GPIOʱ���Լ����ù���ʱ��
    else
        RCC_APB1PeriphClockCmd(p_uart->apb, ENABLE);

	//USART TX
	GPIO_InitStructure.GPIO_Pin   = p_uart->tx_pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������
	GPIO_Init(p_uart->port, &GPIO_InitStructure);
	
	//USART RX
	GPIO_InitStructure.GPIO_Pin  = p_uart->rx_pin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(p_uart->port, &GPIO_InitStructure);  
	
	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = p_uart->irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; 	 //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;		 //IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure); //����ָ���Ĳ�����ʼ��VIC�Ĵ���
	
	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b; //�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;      //һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;         //����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //�շ�ģʽ
	
	USART_Init(p_uart->pUart,&USART_InitStructure); //��ʼ������
	USART_ITConfig(p_uart->pUart, USART_IT_RXNE, ENABLE);//�����ж�
	USART_Cmd(p_uart->pUart, ENABLE); 				   //ʹ�ܴ��� 
}

/****************************************************************************************
�Ӵ����ж�������
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
�򴮿���д������
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
���Դ��ڴ�ӡ����
****************************************************************************************/
void InfoPrintf (char *fmt, ...)
{
    char buffer[CMD_BUFFER_LEN];  // CMD_BUFFER_LEN�����Լ������ 
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
�������ڴ�ӡ����
****************************************************************************************/
void BluetoothPrintf (char *fmt, ...)
{
    char buffer[CMD_BUFFER_LEN];  // CMD_BUFFER_LEN�����Լ������ 
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
�������ݴ�ӡ
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
 ����1�жϷ������
****************************************************************************************/
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	OSIntEnter();    
	usart_irq(DEBUG_DEV);
	OSIntExit();  											 
} 
/****************************************************************************************
 ����2�жϷ������
****************************************************************************************/
void USART2_IRQHandler(void)                	//����2�жϷ������
{
	OSIntEnter();    
	usart_irq(BLUE_TOOTH_DEV);
	OSIntExit();  											 
} 
/****************************************************************************************
 ����3�жϷ������
****************************************************************************************/
void USART3_IRQHandler(void)                	//����3�жϷ������
{
	OSIntEnter();    
	usart_irq(OTHER_DEV);
	OSIntExit();  											 
} 

