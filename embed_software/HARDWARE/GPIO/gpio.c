#include "includes.h"					//ucos ʹ��	  
#include "main.h"
#include "gpio.h"

/****************************************************************************************
 ������GPIO��ʼ��
****************************************************************************************/
void bluetooth_gpio_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��PC�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;		     	//PC.6 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//�����趨������ʼ��GPIOB.5
	GPIO_SetBits(GPIOC,GPIO_Pin_6);						 	//PB.5 �����

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	    		//PC.7 �˿�����,��������
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU; 		 	//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				//������� ��IO���ٶ�Ϊ50MHz
}

/****************************************************************************************
 PWM��GPIO��ʼ��
****************************************************************************************/
void pwm_gpio_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��PC�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;		     	//PC.0 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//�����趨������ʼ��GPIOC.0
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);						//PC.0 �����

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;		     	//PC.1 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//�����趨������ʼ��GPIOC.1
	GPIO_ResetBits(GPIOC,GPIO_Pin_1);						//PC.1 �����

	//FAN+
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4; 			//PC.4 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//�����趨������ʼ��GPIOC.4
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);						//PC.4 �����

	//FAN-
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5; 			//PC.5 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//�����趨������ʼ��GPIOC.5
	GPIO_ResetBits(GPIOC,GPIO_Pin_5);						//PC.5 �����
}

/****************************************************************************************
 RFID��GPIO��ʼ��
****************************************************************************************/
void rfid_gpio_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB , ENABLE);	//ʹ��PC�˿�ʱ��

	//port 0
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;				//PA6: MISO
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7|GPIO_Pin_5;  // PA7: MOSI;  PA5:CLK
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_11; 			//PA11: RST
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_1); 
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4; 			//PA4: CS
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//port 1
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15|GPIO_Pin_14; //PB15: RST ; PB14: CS
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_15); 
}
/****************************************************************************************
 LED��GPIO��ʼ��
****************************************************************************************/
void led_gpio_init(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//ʹ��PC�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;		     	//PC.2 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//�����趨������ʼ��GPIOC.2
	GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);							//PC.6 �����


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	//ʹ��PC�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;		     	//PC.2 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					//�����趨������ʼ��GPIOC.2
	GPIO_ResetBits(GPIOD,GPIO_Pin_13);							//PC.6 �����
}

void Charge_Gpio_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��PA�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;		     	//PC.2 �˿�����
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU; 		//�������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//�����趨������ʼ��GPIOC.2	
}


