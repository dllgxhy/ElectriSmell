#include "includes.h"					//ucos 使用
#include "usart.h"
#include "timer.h"
#include "rtc.h"
#include "stdlib.h"
#include "sys.h"
#include "flash.h"
#include "delay.h"
#include "motor.h"
#include "main.h"
#include "bluetooth.h"

extern OS_FLAG_GRP *TaskCheckOSFlag;



/****************************************************************************************
 定时器初始化
****************************************************************************************/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
** 函数名称: TIMER_Init
** 功能描述: 定时器4初始化配置
** 参数描述：无
** 作  　者: Dream
** 日　  期: 2011年6月20日
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Motor_Init_TIMER(void)
{
	TIM_TimeBaseInitTypeDef	 TIM_BaseInitStructure;			//定义一个定时器结构体变量

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   	//使能定时器4，重要！！

	TIM_DeInit(TIM4);                              			//将IM2定时器初始化位复位值

	TIM_InternalClockConfig(TIM4); 							//配置 TIM4 内部时钟
	   
//	TIM_BaseInitStructure.TIM_Period = 1000; 				//设置自动重载寄存器值为最大值	0~65535之间  1000000/1000=1000us=1ms													
															//TIM_Period（TIM1_ARR）=7200，计数器向上计数到7200后产生更新事件，
															//计数值归零 也就是 1MS产生更新事件一次
	TIM_BaseInitStructure.TIM_Period = 9999;
//	TIM_BaseInitStructure.TIM_Prescaler = 71;  				//自定义预分频系数为0，即定时器的时钟频率为72M提供给定时器的时钟	0~65535之间
															//设置预分频器分频系数0
	TIM_BaseInitStructure.TIM_Prescaler = 16;
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //时钟分割为0
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   
															//TIM向上计数模式 从0开始向上计数，计数到1000后产生更新事件
	TIM_TimeBaseInit(TIM4, &TIM_BaseInitStructure); 		//根据指定参数初始化TIM时间基数寄存器	
      
 	TIM_ARRPreloadConfig(TIM4, ENABLE);						//使能TIMx在 ARR 上的预装载寄存器 

	TIM_Cmd(TIM4, ENABLE); 		//TIM4总开关：开启 
}


/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
** 函数名称: PWM_Init
** 功能描述: 配置PWM通道及占空比
** 参数描述：Dutyfactor 定义占空比大小
** 作  　者: Dream
** 日　  期: 2011年6月20日
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Motor_Init_PWM(uint16_t Dutyfactor)
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
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR1 上的预装载寄存器

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR2 上的预装载寄存器

    TIM_OC3Init(TIM4, &TIM_OCInitStructure);				//根据参数初始化PWM寄存器    
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//使能 TIMx在 CCR3 上的预装载寄存器

    TIM_CtrlPWMOutputs(TIM4,ENABLE);  						//设置TIM4 的PWM 输出为使能  
}

/****************************************************************************************
 Motor PWM用GPIO初始化
****************************************************************************************/
static void Motor_Pwm_Gpio_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能PC端口时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 ; 			
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//复用功能输出推挽
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//配置端口速度为50M
  	GPIO_Init(GPIOB, &GPIO_InitStructure);				   	//将端口GPIOB进行初始化配置
	GPIO_ResetBits(GPIOB,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);
}

/****************************************************************************************
Motor 初始化函数
***************************************************************************************/
void Motor_Init(void)
{
	uint16_t Dutyfactor = 0;
	Motor_Init_TIMER();
	Motor_Init_PWM(Dutyfactor);
	Motor_Pwm_Gpio_Init();

<<<<<<< HEAD
#ifdef RESET_MOTOR_POSITION 
	Reset_Motor_Position();
=======
#ifdef RESET_MOTOR_TO_90 
	Motor_change_angle_to_90();
>>>>>>> 84223f3ea0a6e5eeb048a3177ac34ec8b689dfdc
#endif
}
static void motor_reset2zero(MOTOR_ID motorid,u16 fromangle)
{
	u16 angle_target = 0x00;
	if(fromangle > 0x00)
	{
		while(fromangle --)
		{
			angle_target = (u16)(5.56*fromangle + 249);
			switch(motorid)
			{
				case MOTOR_ID1:
					TIM_SetCompare1(TIM4,angle_target);
					break;
				case MOTOR_ID2:
					TIM_SetCompare2(TIM4,angle_target);
					break;
				case MOTOR_ID3:
					TIM_SetCompare3(TIM4,angle_target);
					break;
				default:
					break;
			} 	
			delay_ms(5);	
		}
		delay_ms(20);
	}
}

void Motor_change_angle_to(MOTOR_ID id,u16 angle)
{
	static uint16_t angle_tmp_id1 = 0x00;
	static uint16_t angle_tmp_id2 = 0x00;
	static uint16_t angle_tmp_id3 = 0x00;
	uint16_t angle_target = 0x00;

	switch(id)
	{
		case  MOTOR_ID1:
			motor_reset2zero(MOTOR_ID1,angle_tmp_id1);
			angle_tmp_id1 = 0x00;
			while(++ angle_tmp_id1)
			{			
//				angle_target = (uint16_t)(5.56*angle_tmp_id1 + 249);
				angle_target = (uint16_t)(5.5*angle_tmp_id1 + 249);
			   	TIM_SetCompare1(TIM4,angle_target);
				delay_ms(2);
				if(angle_tmp_id1 > angle)
				{			
					TIM_SetCompare1(TIM4,0);
					break;
				}
			}
			break;

		case  MOTOR_ID2:
			motor_reset2zero(MOTOR_ID2,angle_tmp_id2);
			angle_tmp_id2 = 0x00;
			while(++ angle_tmp_id2 )
			{			
//				angle_target = (unsigned short)(4.2*angle_tmp_id2 + 410);
				angle_target = (uint16_t)(5.5*angle_tmp_id2 + 249);
			   	TIM_SetCompare2(TIM4,angle_target);
				delay_ms(2);
				if(angle_tmp_id2 > angle)
				{
					TIM_SetCompare2(TIM4,0);			
					break;
				}
			}
			break;

		case  MOTOR_ID3:
			motor_reset2zero(MOTOR_ID3,angle_tmp_id3);
			angle_tmp_id3 = 0x00;
			while(++ angle_tmp_id3)
			{			
//				angle_target = (unsigned short)(4.2*angle_tmp_id3 + 410);
				angle_target = (uint16_t)(5.5*angle_tmp_id3 + 249);
			   	TIM_SetCompare3(TIM4,angle_target);
				delay_ms(2);
				if(angle_tmp_id3 > angle)
				{
					TIM_SetCompare3(TIM4,0);			
					break;
				}
			}

			break;
		default:
			break;
	}
}
/****************************************************************************************
舵机转到90度
****************************************************************************************/

void Reset_Motor_Position(void)
{
	Motor_change_angle_to(MOTOR_ID1,DEFAULT_MOTOR_RESET_ANGLE);
	Motor_change_angle_to(MOTOR_ID2,DEFAULT_MOTOR_RESET_ANGLE);
	Motor_change_angle_to(MOTOR_ID3,DEFAULT_MOTOR_RESET_ANGLE);
}


/****************************************************************************************
 舵机数据处理任务
****************************************************************************************/
void motor_proc_handle(void)
{
	if( flush_motor_flag )
	{	//转动舵机
		Motor_change_angle_to(MOTOR_ID1,param.group1.motor_angle);
		Motor_change_angle_to(MOTOR_ID2,param.group2.motor_angle);
		Motor_change_angle_to(MOTOR_ID3,param.group3.motor_angle);
		flush_motor_flag = 0;
		//舵机到位后，风扇开始转动
		flush_fan_flag   = 1;
	}
}
/****************************************************************************************
 舵机数据处理任务
****************************************************************************************/
void motor_proc_task(void *pdata)
{
	u8  err;
	pdata = pdata;

    while(1)
    {
    	motor_proc_handle();	//蓝牙数据消息处理函数
			
        OSTimeDly(5);
		OSFlagPost(TaskCheckOSFlag,OSFLAG_MOTOR_PROC_TASK,OS_FLAG_SET,&err);
    }
}


