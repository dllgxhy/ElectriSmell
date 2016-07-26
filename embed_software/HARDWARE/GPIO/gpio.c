#include "includes.h"					//ucos 使用	  
#include "main.h"
#include "gpio.h"

/****************************************************************************************
 蓝牙用GPIO初始化
****************************************************************************************/
void bluetooth_gpio_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能PC端口时钟

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6;		     	//PC.6 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOB.5
	GPIO_SetBits(GPIOC,GPIO_Pin_6);						 	//PB.5 输出高

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;	    		//PC.7 端口配置,上拉输入
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU; 		 	//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);	  				//推挽输出 ，IO口速度为50MHz
}

/****************************************************************************************
 PWM用GPIO初始化
****************************************************************************************/
void pwm_gpio_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能PC端口时钟

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0;		     	//PC.0 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOC.0
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);						//PC.0 输出低

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;		     	//PC.1 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOC.1
	GPIO_ResetBits(GPIOC,GPIO_Pin_1);						//PC.1 输出低

	//FAN+
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4; 			//PC.4 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOC.4
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);						//PC.4 输出低

	//FAN-
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5; 			//PC.5 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOC.5
	GPIO_ResetBits(GPIOC,GPIO_Pin_5);						//PC.5 输出低
}

/****************************************************************************************
 RFID用GPIO初始化
****************************************************************************************/
void rfid_gpio_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB , ENABLE);	//使能PC端口时钟

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
 LED用GPIO初始化
****************************************************************************************/
void led_gpio_init(void)
{

	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	//使能PC端口时钟

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;		     	//PC.2 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//根据设定参数初始化GPIOC.2
	GPIO_ResetBits(GPIOC,GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);							//PC.6 输出低


	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	//使能PC端口时钟
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;		     	//PC.2 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 		//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//IO口速度为50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					//根据设定参数初始化GPIOC.2
	GPIO_ResetBits(GPIOD,GPIO_Pin_13);							//PC.6 输出低
}

void Charge_Gpio_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能PA端口时钟
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;		     	//PC.2 端口配置
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU; 		//推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//根据设定参数初始化GPIOC.2	
}


