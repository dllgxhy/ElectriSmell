#include "includes.h"					//ucos ʹ��	  
#include "timer.h"

static TIMER_INFO timer_info_list[] = {
	{TIM2,TIM2_IRQn,RCC_APB1Periph_TIM2,113,72000,0},		//pwm1
	{TIM3,TIM3_IRQn,RCC_APB1Periph_TIM3,113,72000,0},		//pwm2
	{TIM4,TIM4_IRQn,RCC_APB1Periph_TIM4,100,72000,0},		//ʱ���ʱ��,100msһ��
};

static unsigned short keep_time = 0;
static unsigned char  keep_flag = 0;
/****************************************************************************************
 ���ȳ���ʱ������
****************************************************************************************/
void set_fan_keep_time(unsigned short tim )
{
#if OS_CRITICAL_METHOD == 3u                     /* Allocate storage for CPU status register           */
	OS_CPU_SR  cpu_sr = 0u;
#endif
    OS_ENTER_CRITICAL();
	if( keep_time >= tim )
	{
		OS_EXIT_CRITICAL();
		return;
	}
	keep_time = tim;
	keep_flag = 1;
    OS_EXIT_CRITICAL();
}

/****************************************************************************************
 ��ʱ����ʼ��
****************************************************************************************/
void timer_init(u16 dev)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIMER_INFO*      p_timer = &timer_info_list[dev];

	RCC_APB1PeriphClockCmd(p_timer->apb, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIMX��ʼ��
	TIM_TimeBaseStructure.TIM_Period    = p_timer->arr - 1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler = p_timer->psc - 1; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(p_timer->timer, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(p_timer->timer,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIMx�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = p_timer->irq;  //TIMx�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���

	TIM_Cmd(p_timer->timer, ENABLE);  //ʹ��TIMx					 
}

/****************************************************************************************
 ��ʱ�����³�ʼ��
****************************************************************************************/
void timer_reinit(u16 dev,u32 arr, u32 psc,u32 keep)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIMER_INFO*      p_timer = &timer_info_list[dev];

	RCC_APB1PeriphClockCmd(p_timer->apb, ENABLE); //ʱ��ʹ��

	TIM_Cmd(p_timer->timer, DISABLE);  //ʹ��TIMx					 

	//��ʱ��TIMX��ʼ��
	p_timer->arr  = arr;
	p_timer->psc  = psc;
	p_timer->keep = keep * (36000000 / (arr * psc));
	TIM_TimeBaseStructure.TIM_Period    = p_timer->arr - 1; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler = p_timer->psc - 1; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(p_timer->timer, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	TIM_ITConfig(p_timer->timer,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIMx�ж�,��������ж�

	TIM_Cmd(p_timer->timer, ENABLE);  //ʹ��TIMx					 
}

/****************************************************************************************
 ��ʱ��2�жϷ������
****************************************************************************************/
void TIM2_IRQHandler(void)   //TIM2�ж�
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM2�����жϷ������
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
		if( keep_flag )
			PCout(0)  = !PCout(0);
		else
			PCout(0)  = 0;
	}
}

/****************************************************************************************
 ��ʱ��3�жϷ������
****************************************************************************************/
void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���TIM3�����жϷ������
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
		if( keep_flag )
			PCout(1) = !PCout(1);
		else
			PCout(1) = 0;
	}
}

/****************************************************************************************
 ��ʱ��4�жϷ������
 ʱ�䶨ʱ��
****************************************************************************************/
void TIM4_IRQHandler(void)   //TIM4�ж�
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //���TIM4�����жϷ������
	{
		if( keep_time )
		{
			PCout(4) = 1;	//���ȿ�ʼת��
			//keep_time--;
		}
		else
		{
			PCout(4) = 0;	//����ֹͣת��
			//keep_flag = 0;
		}
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //���TIMx�����жϱ�־ 
	}
}

