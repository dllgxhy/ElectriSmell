#include "includes.h"					//ucos 使用	  
#include "timer.h"

static TIMER_INFO timer_info_list[] = {
	{TIM2,TIM2_IRQn,RCC_APB1Periph_TIM2,113,72000,0},		//pwm1
	{TIM3,TIM3_IRQn,RCC_APB1Periph_TIM3,113,72000,0},		//pwm2
	{TIM4,TIM4_IRQn,RCC_APB1Periph_TIM4,100,72000,0},		//时间计时器,100ms一次
};

static unsigned short keep_time = 0;
static unsigned char  keep_flag = 0;
/****************************************************************************************
 风扇持续时间设置
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
 定时器初始化
****************************************************************************************/
void timer_init(u16 dev)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIMER_INFO*      p_timer = &timer_info_list[dev];

	RCC_APB1PeriphClockCmd(p_timer->apb, ENABLE); //时钟使能
	
	//定时器TIMX初始化
	TIM_TimeBaseStructure.TIM_Period    = p_timer->arr - 1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = p_timer->psc - 1; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(p_timer->timer, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(p_timer->timer,TIM_IT_Update,ENABLE ); //使能指定的TIMx中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = p_timer->irq;  //TIMx中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器

	TIM_Cmd(p_timer->timer, ENABLE);  //使能TIMx					 
}

/****************************************************************************************
 定时器重新初始化
****************************************************************************************/
void timer_reinit(u16 dev,u32 arr, u32 psc,u32 keep)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIMER_INFO*      p_timer = &timer_info_list[dev];

	RCC_APB1PeriphClockCmd(p_timer->apb, ENABLE); //时钟使能

	TIM_Cmd(p_timer->timer, DISABLE);  //使能TIMx					 

	//定时器TIMX初始化
	p_timer->arr  = arr;
	p_timer->psc  = psc;
	p_timer->keep = keep * (36000000 / (arr * psc));
	TIM_TimeBaseStructure.TIM_Period    = p_timer->arr - 1; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = p_timer->psc - 1; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(p_timer->timer, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(p_timer->timer,TIM_IT_Update,ENABLE ); //使能指定的TIMx中断,允许更新中断

	TIM_Cmd(p_timer->timer, ENABLE);  //使能TIMx					 
}

/****************************************************************************************
 定时器2中断服务程序
****************************************************************************************/
void TIM2_IRQHandler(void)   //TIM2中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查TIM2更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  //清除TIMx更新中断标志 
		if( keep_flag )
			PCout(0)  = !PCout(0);
		else
			PCout(0)  = 0;
	}
}

/****************************************************************************************
 定时器3中断服务程序
****************************************************************************************/
void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志 
		if( keep_flag )
			PCout(1) = !PCout(1);
		else
			PCout(1) = 0;
	}
}

/****************************************************************************************
 定时器4中断服务程序
 时间定时器
****************************************************************************************/
void TIM4_IRQHandler(void)   //TIM4中断
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查TIM4更新中断发生与否
	{
		if( keep_time )
		{
			PCout(4) = 1;	//风扇开始转动
			//keep_time--;
		}
		else
		{
			PCout(4) = 0;	//风扇停止转动
			//keep_flag = 0;
		}
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志 
	}
}

