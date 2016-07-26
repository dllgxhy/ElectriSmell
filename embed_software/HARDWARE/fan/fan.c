#include "includes.h"					//ucos 使用
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "rtc.h"
#include "stdlib.h"
#include "sys.h"
#include "flash.h"
#include "delay.h"
#include "main.h"
#include "fan.h"
#include "motor.h"
#include "bluetooth.h"
extern OS_FLAG_GRP *TaskCheckOSFlag;
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
** 函数名称: TIMER_Init
** 功能描述: 定时器3初始化配置
** 参数描述：无
** 作  　者: Dream
** 日　  期: 2011年6月20日
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Fan_Init_TIMER(void)
{
	TIM_TimeBaseInitTypeDef	 TIM_BaseInitStructure;			//定义一个定时器结构体变量

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);   	//使能定时器4，重要！！

	TIM_DeInit(TIM3);                              			//将IM2定时器初始化位复位值

	TIM_InternalClockConfig(TIM3); 							//配置 TIM4 内部时钟
	   
	TIM_BaseInitStructure.TIM_Period = 7200-1; 				//设置自动重载寄存器值为最大值	0~65535之间  1000000/1000=1000us=1ms													
															//TIM_Period（TIM1_ARR）=7200，计数器向上计数到7200后产生更新事件，
															//计数值归零 也就是 1MS产生更新事件一次
	TIM_BaseInitStructure.TIM_Prescaler = 1000;  				//自定义预分频系数为0，即定时器的时钟频率为72M提供给定时器的时钟	0~65535之间
															//设置预分频器分频系数0
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分割为0
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   
															//TIM向上计数模式 从0开始向上计数，计数到1000后产生更新事件
	TIM_TimeBaseInit(TIM3, &TIM_BaseInitStructure); 		//根据指定参数初始化TIM时间基数寄存器	
      
 	TIM_ARRPreloadConfig(TIM3, ENABLE);						//使能TIMx在 ARR 上的预装载寄存器 

	TIM_Cmd(TIM3, ENABLE); 		//TIM4总开关：开启 
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
** 函数名称: PWM_Init
** 功能描述: 配置PWM通道及占空比
** 参数描述：Dutyfactor 定义占空比大小
** 作  　者: Dream
** 日　  期: 2011年6月20日
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Fan_Init_PWM(uint16_t Dutyfactor)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;					//定义一个通道输出结构


	TIM_OCStructInit(&TIM_OCInitStructure);					//设置缺省值

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	   	//PWM 模式 1 输出 	
	TIM_OCInitStructure.TIM_Pulse = Dutyfactor; 			//设置占空比，占空比=(CCRx/ARR)*100%或(TIM_Pulse/TIM_Period)*100%
															//PWM的输出频率为Fpwm=72M/7200=1Mhz；  
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
															//TIM 输出比较极性高   	    
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;    
															//使能输出状态  需要PWM输出才需要这行代码
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR1 上的预装载寄存器

    TIM_OC2Init(TIM3, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR2 上的预装载寄存器

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR3 上的预装载寄存器

    TIM_CtrlPWMOutputs(TIM3,ENABLE);  						//设置TIM4 的PWM 输出为使能  
}

/****************************************************************************************
 Fan PWM用GPIO初始化
****************************************************************************************/
static void Fan_Pwm_Gpio_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//使能PA端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7  ; 			
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//复用功能输出推挽
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//配置端口速度为50M
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//将端口GPIOB进行初始化配置
	GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7);


   	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能PB端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ; 			
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//复用功能输出推挽
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//配置端口速度为50M
  	GPIO_Init(GPIOB, &GPIO_InitStructure);				   	//将端口GPIOB进行初始化配置
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);
}

/*****************************************************************************************
*****************************************************************************************/
void Fan_Init(void)
{
	uint16_t Dutyfactor = 0;
	Fan_Init_TIMER();
	Fan_Init_PWM(Dutyfactor);		
	Fan_Pwm_Gpio_Init();
}


uint16_t __fan_level = 0x00;
uint16_t Fan_Get_LevelData(u8 fan_level)
{
	switch(fan_level)
	{
		case FAN_LEVEL1:
			__fan_level = 0xffffffff;
			break;
		case FAN_LEVEL2:
			__fan_level = 0xffffffff;
			break;
		case FAN_LEVEL3:
			__fan_level = 0xffffffff;
			break;
		default :
			break ;
	}
	return __fan_level;
}

void start_fan_work(MOTOR_ID id,uint16_t fan_time_s,FAN_LEVEL fan_level)
{
	uint16_t fan_level_data_tmp = 0x00;
	fan_level_data_tmp = Fan_Get_LevelData(fan_level);
	switch(id)
	{
		case MOTOR_ID1:
			TIM_SetCompare1(TIM3,fan_level_data_tmp);
			delay_ms(fan_time_s);
			TIM_SetCompare1(TIM3,0);
			break;
		case MOTOR_ID2:
			TIM_SetCompare2(TIM3,fan_level_data_tmp);
			delay_ms(fan_time_s);
			TIM_SetCompare2(TIM3,0);

			break;
		case MOTOR_ID3:
			TIM_SetCompare3(TIM3,fan_level_data_tmp);
			delay_ms(fan_time_s);
			TIM_SetCompare3(TIM3,0);

			break;
		default:
			break;
	}	
}
/****************************************************************************************
 风扇数据消息处理函数
****************************************************************************************/
void fan_proc_handle(void)
{
	u16 time = 0;
	u8  flag = 0x07;
	if( flush_fan_flag == 1 )
	{
		flush_stop_flag = 1;
		TIM_SetCompare1(TIM3,Fan_Get_LevelData(param.group1.fan_level));
		TIM_SetCompare2(TIM3,Fan_Get_LevelData(param.group2.fan_level));
		TIM_SetCompare3(TIM3,Fan_Get_LevelData(param.group3.fan_level));
		while(flag)
		{
			time++;
			if( time > param.group1.fan_time )
			{
				TIM_SetCompare1(TIM3,0);
				flag &= 0xfe;
			}

			if( time > param.group2.fan_time )
			{
				TIM_SetCompare2(TIM3,0);
				flag &= 0xfd;
			}
			if( time > param.group3.fan_time )
			{
				TIM_SetCompare3(TIM3,0);
				flag &= 0xfb;
			} 
		
			
			if( flush_stop_flag == 2 )
			{
				TIM_SetCompare1(TIM3,0);
				TIM_SetCompare2(TIM3,0);
				TIM_SetCompare3(TIM3,0);
				break;
			} 
			OSTimeDly(200);
		}
#ifdef RESET_MOTOR_POSITION
		Reset_Motor_Position(); //风扇停止转动后，将舵机转到90度即可
#endif
		flush_stop_flag = 0;
		flush_fan_flag  = 0;
	}
}
/****************************************************************************************
 风扇数据处理任务
****************************************************************************************/
void fan_proc_task(void *pdata)
{
	u8  err;
	pdata = pdata;

    while(1)
    {
    	fan_proc_handle();	//风扇数据消息处理函数
			
        OSTimeDly(5);
		OSFlagPost(TaskCheckOSFlag,OSFLAG_FAN_PROC_TASK,OS_FLAG_SET,&err);
    }
}



