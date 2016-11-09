#include "includes.h"
#include "delay.h"
#include "main.h"
#include "usart.h"	
#include "gpio.h"
#include "timer.h"
#include "finsh.h"
#include "motor.h"
#include "fan.h"
#include "wkup.h"
#include "rtc.h"

/////////////////////////UCOSII任务堆栈设置///////////////////////////////////
/***** START 任务 *****/
#define START_TASK_PRIO      			5   //设置任务优先级,开始任务的优先级设置为最低
#define START_STK_SIZE  				64  //设置任务堆栈大小
OS_STK START_TASK_STK[START_STK_SIZE] = {0};//创建任务堆栈空间	
void start_task(void *pdata);	            //任务函数接口
 			   
/***** 蓝牙数据处理任务 *****/
#define BLUETOOTH_PROC_TASK_PRIO     7                      //设置任务优先级
#define BLUETOOTH_PROC_STK_SIZE      512                    //设置任务堆栈大小
OS_STK BLUETOOTH_PROC_TASK_STK[BLUETOOTH_PROC_STK_SIZE];    //创建任务堆栈空间	
extern void bluetooth_proc_task(void *pdata);               //任务函数接口

/***** 蓝牙数据处理任务 *****/
#define BLUETOOTH_RECEIVE_TASK_PRIO     6                       //设置任务优先级
#define BLUETOOTH_RECEIVE_STK_SIZE      512                     //设置任务堆栈大小
OS_STK BLUETOOTH_RECEIVE_TASK_STK[BLUETOOTH_RECEIVE_STK_SIZE];  //创建任务堆栈空间	
extern void bluetooth_receive_task(void *pdata);                //任务函数接口

/***** rfid数据处理任务 *****/
#define RFID_PROC_TASK_PRIO     8                       //设置任务优先级
#define RFID_PROC_STK_SIZE      512                     //设置任务堆栈大小
OS_STK RFID_PROC_TASK_STK[RFID_PROC_STK_SIZE];          //创建任务堆栈空间	
extern void rfid_proc_task(void *pdata);                //任务函数接口

/***** motor数据处理任务 *****/
#define MOTOR_PROC_TASK_PRIO     9                       //设置任务优先级
#define MOTOR_PROC_STK_SIZE      256                     //设置任务堆栈大小
OS_STK MOTOR_PROC_TASK_STK[MOTOR_PROC_STK_SIZE];          //创建任务堆栈空间	
extern void motor_proc_task(void *pdata);                //任务函数接口

/***** motor数据处理任务 *****/
#define FAN_PROC_TASK_PRIO     10                       //设置任务优先级
#define FAN_PROC_STK_SIZE      256                     //设置任务堆栈大小
OS_STK FAN_PROC_TASK_STK[FAN_PROC_STK_SIZE];          //创建任务堆栈空间	
extern void fan_proc_task(void *pdata);                //任务函数接口

/**** 蓝牙任务消息队列 ****/
#define MaxBluetoothTaskMsgQTab        	8
void *BluetoothTaskMsgQTab        		[MaxBluetoothTaskMsgQTab];
OS_EVENT    *BluetoothMsgQ;

OS_FLAG_GRP *TaskCheckOSFlag;
/****************************************************************************************
 主程序
****************************************************************************************/

int main(void)
{
	delay_init();	     //延时初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级

	bluetooth_gpio_init();
	
    uart_init(DEBUG_DEV,115200);
    uart_init(BLUE_TOOTH_DEV,9600);
	led_gpio_init();
	Charge_Gpio_Init();
	Motor_Init();
	Fan_Init();
	RTC_Init();
	WKUP_Init();
    
    InfoPrintf("-------- main start --------\r\n");
	OSInit();   
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//创建起始任务
	OSStart();
}



/****************************************************************************************
 启动任务
****************************************************************************************/
void start_task(void *pdata)
{
    u8          err          = 0;
    u16         TaskCheckCnt = 0;
    OS_CPU_SR   cpu_sr       = 0;
	OS_FLAGS    run_flag;
	pdata = pdata; 
	OSStatInit();					//初始化统计任务.这里会延时1秒钟左右	
 	OS_ENTER_CRITICAL();			//进入临界区(无法被中断打断)    

	BluetoothMsgQ      = OSQCreate(BluetoothTaskMsgQTab,      MaxBluetoothTaskMsgQTab);

 	OSTaskCreate(bluetooth_proc_task,   (void *)0,(OS_STK*)&BLUETOOTH_PROC_TASK_STK[BLUETOOTH_PROC_STK_SIZE-1],      BLUETOOTH_PROC_TASK_PRIO);
 	OSTaskCreate(bluetooth_receive_task,(void *)0,(OS_STK*)&BLUETOOTH_RECEIVE_TASK_STK[BLUETOOTH_RECEIVE_STK_SIZE-1],BLUETOOTH_RECEIVE_TASK_PRIO);

 	OSTaskCreate(fan_proc_task,(void *)0,(OS_STK*)&FAN_PROC_TASK_STK[FAN_PROC_STK_SIZE-1],FAN_PROC_TASK_PRIO);
 	OSTaskCreate(motor_proc_task,(void *)0,(OS_STK*)&MOTOR_PROC_TASK_STK[MOTOR_PROC_STK_SIZE-1],MOTOR_PROC_TASK_PRIO);

    //finsh_system_init();

 	TaskCheckOSFlag = OSFlagCreate(0,&err);

	OS_EXIT_CRITICAL();				//退出临界区(可以被中断打断)

    while(1)
    {
		run_flag = OSFlagAccept(TaskCheckOSFlag,OSFLAG_BLUETOOTH_PROC_TASK|OSFLAG_BLUETOOTH_RECEIVE_TASK|OSFLAG_FAN_PROC_TASK|OSFLAG_MOTOR_PROC_TASK,\
			OS_FLAG_WAIT_SET_ALL | OS_FLAG_CONSUME,&err);
		if(OS_ERR_NONE != err)
		{
			TaskCheckCnt++;
			if(TaskCheckCnt>500)
			{	
				TaskCheckCnt=0;
				InfoPrintf("task time out,will reset,0x%x\r\n",run_flag);
				OSTimeDly(2);
				NVIC_SystemReset();// 复位
			}
		}
		else
			TaskCheckCnt = 0;
		OSTimeDly(100);
 	}
}

