#include "includes.h"					//ucos ʹ��
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
** ��������: TIMER_Init
** ��������: ��ʱ��3��ʼ������
** ������������
** ��  ����: Dream
** �ա�  ��: 2011��6��20��
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Fan_Init_TIMER(void)
{
	TIM_TimeBaseInitTypeDef	 TIM_BaseInitStructure;			//����һ����ʱ���ṹ�����

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);   	//ʹ�ܶ�ʱ��4����Ҫ����

	TIM_DeInit(TIM3);                              			//��IM2��ʱ����ʼ��λ��λֵ

	TIM_InternalClockConfig(TIM3); 							//���� TIM4 �ڲ�ʱ��
	   
	TIM_BaseInitStructure.TIM_Period = 7200-1; 				//�����Զ����ؼĴ���ֵΪ���ֵ	0~65535֮��  1000000/1000=1000us=1ms													
															//TIM_Period��TIM1_ARR��=7200�����������ϼ�����7200����������¼���
															//����ֵ���� Ҳ���� 1MS���������¼�һ��
	TIM_BaseInitStructure.TIM_Prescaler = 1000;  				//�Զ���Ԥ��Ƶϵ��Ϊ0������ʱ����ʱ��Ƶ��Ϊ72M�ṩ����ʱ����ʱ��	0~65535֮��
															//����Ԥ��Ƶ����Ƶϵ��0
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ʱ�ӷָ�Ϊ0
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   
															//TIM���ϼ���ģʽ ��0��ʼ���ϼ�����������1000����������¼�
	TIM_TimeBaseInit(TIM3, &TIM_BaseInitStructure); 		//����ָ��������ʼ��TIMʱ������Ĵ���	
      
 	TIM_ARRPreloadConfig(TIM3, ENABLE);						//ʹ��TIMx�� ARR �ϵ�Ԥװ�ؼĴ��� 

	TIM_Cmd(TIM3, ENABLE); 		//TIM4�ܿ��أ����� 
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
** ��������: PWM_Init
** ��������: ����PWMͨ����ռ�ձ�
** ����������Dutyfactor ����ռ�ձȴ�С
** ��  ����: Dream
** �ա�  ��: 2011��6��20��
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Fan_Init_PWM(uint16_t Dutyfactor)
{
	TIM_OCInitTypeDef  TIM_OCInitStructure;					//����һ��ͨ������ṹ


	TIM_OCStructInit(&TIM_OCInitStructure);					//����ȱʡֵ

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	   	//PWM ģʽ 1 ��� 	
	TIM_OCInitStructure.TIM_Pulse = Dutyfactor; 			//����ռ�ձȣ�ռ�ձ�=(CCRx/ARR)*100%��(TIM_Pulse/TIM_Period)*100%
															//PWM�����Ƶ��ΪFpwm=72M/7200=1Mhz��  
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
															//TIM ����Ƚϼ��Ը�   	    
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;    
															//ʹ�����״̬  ��ҪPWM�������Ҫ���д���
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);				//���ݲ�����ʼ��PWM�Ĵ���    
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);	   	//ʹ�� TIMx�� CCR1 �ϵ�Ԥװ�ؼĴ���

    TIM_OC2Init(TIM3, &TIM_OCInitStructure);				//���ݲ�����ʼ��PWM�Ĵ���    
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);	   	//ʹ�� TIMx�� CCR2 �ϵ�Ԥװ�ؼĴ���

    TIM_OC3Init(TIM3, &TIM_OCInitStructure);				//���ݲ�����ʼ��PWM�Ĵ���    
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);	   	//ʹ�� TIMx�� CCR3 �ϵ�Ԥװ�ؼĴ���

    TIM_CtrlPWMOutputs(TIM3,ENABLE);  						//����TIM4 ��PWM ���Ϊʹ��  
}

/****************************************************************************************
 Fan PWM��GPIO��ʼ��
****************************************************************************************/
static void Fan_Pwm_Gpio_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��PA�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7  ; 			
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//���ù����������
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//���ö˿��ٶ�Ϊ50M
  	GPIO_Init(GPIOA, &GPIO_InitStructure);				   	//���˿�GPIOB���г�ʼ������
	GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7);


   	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��PB�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 ; 			
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//���ù����������
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//���ö˿��ٶ�Ϊ50M
  	GPIO_Init(GPIOB, &GPIO_InitStructure);				   	//���˿�GPIOB���г�ʼ������
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
 ����������Ϣ������
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
		Reset_Motor_Position(); //����ֹͣת���󣬽����ת��90�ȼ���
#endif
		flush_stop_flag = 0;
		flush_fan_flag  = 0;
	}
}
/****************************************************************************************
 �������ݴ�������
****************************************************************************************/
void fan_proc_task(void *pdata)
{
	u8  err;
	pdata = pdata;

    while(1)
    {
    	fan_proc_handle();	//����������Ϣ������
			
        OSTimeDly(5);
		OSFlagPost(TaskCheckOSFlag,OSFLAG_FAN_PROC_TASK,OS_FLAG_SET,&err);
    }
}



