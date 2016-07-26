#include "includes.h"					//ucos ʹ��
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
 ��ʱ����ʼ��
****************************************************************************************/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
** ��������: TIMER_Init
** ��������: ��ʱ��4��ʼ������
** ������������
** ��  ����: Dream
** �ա�  ��: 2011��6��20��
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Motor_Init_TIMER(void)
{
	TIM_TimeBaseInitTypeDef	 TIM_BaseInitStructure;			//����һ����ʱ���ṹ�����

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);   	//ʹ�ܶ�ʱ��4����Ҫ����

	TIM_DeInit(TIM4);                              			//��IM2��ʱ����ʼ��λ��λֵ

	TIM_InternalClockConfig(TIM4); 							//���� TIM4 �ڲ�ʱ��
	   
//	TIM_BaseInitStructure.TIM_Period = 1000; 				//�����Զ����ؼĴ���ֵΪ���ֵ	0~65535֮��  1000000/1000=1000us=1ms													
															//TIM_Period��TIM1_ARR��=7200�����������ϼ�����7200����������¼���
															//����ֵ���� Ҳ���� 1MS���������¼�һ��
	TIM_BaseInitStructure.TIM_Period = 9999;
//	TIM_BaseInitStructure.TIM_Prescaler = 71;  				//�Զ���Ԥ��Ƶϵ��Ϊ0������ʱ����ʱ��Ƶ��Ϊ72M�ṩ����ʱ����ʱ��	0~65535֮��
															//����Ԥ��Ƶ����Ƶϵ��0
	TIM_BaseInitStructure.TIM_Prescaler = 16;
	TIM_BaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; //ʱ�ӷָ�Ϊ0
	TIM_BaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;   
															//TIM���ϼ���ģʽ ��0��ʼ���ϼ�����������1000����������¼�
	TIM_TimeBaseInit(TIM4, &TIM_BaseInitStructure); 		//����ָ��������ʼ��TIMʱ������Ĵ���	
      
 	TIM_ARRPreloadConfig(TIM4, ENABLE);						//ʹ��TIMx�� ARR �ϵ�Ԥװ�ؼĴ��� 

	TIM_Cmd(TIM4, ENABLE); 		//TIM4�ܿ��أ����� 
}


/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
** ��������: PWM_Init
** ��������: ����PWMͨ����ռ�ձ�
** ����������Dutyfactor ����ռ�ձȴ�С
** ��  ����: Dream
** �ա�  ��: 2011��6��20��
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void Motor_Init_PWM(uint16_t Dutyfactor)
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
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);				//���ݲ�����ʼ��PWM�Ĵ���    
	TIM_OC1PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//ʹ�� TIMx�� CCR1 �ϵ�Ԥװ�ؼĴ���

    TIM_OC2Init(TIM4, &TIM_OCInitStructure);				//���ݲ�����ʼ��PWM�Ĵ���    
	TIM_OC2PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//ʹ�� TIMx�� CCR2 �ϵ�Ԥװ�ؼĴ���

    TIM_OC3Init(TIM4, &TIM_OCInitStructure);				//���ݲ�����ʼ��PWM�Ĵ���    
	TIM_OC3PreloadConfig(TIM4,TIM_OCPreload_Enable);	   	//ʹ�� TIMx�� CCR3 �ϵ�Ԥװ�ؼĴ���

    TIM_CtrlPWMOutputs(TIM4,ENABLE);  						//����TIM4 ��PWM ���Ϊʹ��  
}

/****************************************************************************************
 Motor PWM��GPIO��ʼ��
****************************************************************************************/
static void Motor_Pwm_Gpio_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��PC�˿�ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 ; 			
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	   		//���ù����������
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	   	//���ö˿��ٶ�Ϊ50M
  	GPIO_Init(GPIOB, &GPIO_InitStructure);				   	//���˿�GPIOB���г�ʼ������
	GPIO_ResetBits(GPIOB,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8);
}

/****************************************************************************************
Motor ��ʼ������
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
���ת��90��
****************************************************************************************/

void Reset_Motor_Position(void)
{
	Motor_change_angle_to(MOTOR_ID1,DEFAULT_MOTOR_RESET_ANGLE);
	Motor_change_angle_to(MOTOR_ID2,DEFAULT_MOTOR_RESET_ANGLE);
	Motor_change_angle_to(MOTOR_ID3,DEFAULT_MOTOR_RESET_ANGLE);
}


/****************************************************************************************
 ������ݴ�������
****************************************************************************************/
void motor_proc_handle(void)
{
	if( flush_motor_flag )
	{	//ת�����
		Motor_change_angle_to(MOTOR_ID1,param.group1.motor_angle);
		Motor_change_angle_to(MOTOR_ID2,param.group2.motor_angle);
		Motor_change_angle_to(MOTOR_ID3,param.group3.motor_angle);
		flush_motor_flag = 0;
		//�����λ�󣬷��ȿ�ʼת��
		flush_fan_flag   = 1;
	}
}
/****************************************************************************************
 ������ݴ�������
****************************************************************************************/
void motor_proc_task(void *pdata)
{
	u8  err;
	pdata = pdata;

    while(1)
    {
    	motor_proc_handle();	//����������Ϣ������
			
        OSTimeDly(5);
		OSFlagPost(TaskCheckOSFlag,OSFLAG_MOTOR_PROC_TASK,OS_FLAG_SET,&err);
    }
}


