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

/////////////////////////UCOSII�����ջ����///////////////////////////////////
/***** START ���� *****/
#define START_TASK_PRIO      			5   //�����������ȼ�,��ʼ��������ȼ�����Ϊ���
#define START_STK_SIZE  				64  //���������ջ��С
OS_STK START_TASK_STK[START_STK_SIZE] = {0};//���������ջ�ռ�	
void start_task(void *pdata);	            //�������ӿ�
 			   
/***** �������ݴ������� *****/
#define BLUETOOTH_PROC_TASK_PRIO     7                      //�����������ȼ�
#define BLUETOOTH_PROC_STK_SIZE      512                    //���������ջ��С
OS_STK BLUETOOTH_PROC_TASK_STK[BLUETOOTH_PROC_STK_SIZE];    //���������ջ�ռ�	
extern void bluetooth_proc_task(void *pdata);               //�������ӿ�

/***** �������ݴ������� *****/
#define BLUETOOTH_RECEIVE_TASK_PRIO     6                       //�����������ȼ�
#define BLUETOOTH_RECEIVE_STK_SIZE      512                     //���������ջ��С
OS_STK BLUETOOTH_RECEIVE_TASK_STK[BLUETOOTH_RECEIVE_STK_SIZE];  //���������ջ�ռ�	
extern void bluetooth_receive_task(void *pdata);                //�������ӿ�

/***** rfid���ݴ������� *****/
#define RFID_PROC_TASK_PRIO     8                       //�����������ȼ�
#define RFID_PROC_STK_SIZE      512                     //���������ջ��С
OS_STK RFID_PROC_TASK_STK[RFID_PROC_STK_SIZE];          //���������ջ�ռ�	
extern void rfid_proc_task(void *pdata);                //�������ӿ�

/***** motor���ݴ������� *****/
#define MOTOR_PROC_TASK_PRIO     9                       //�����������ȼ�
#define MOTOR_PROC_STK_SIZE      256                     //���������ջ��С
OS_STK MOTOR_PROC_TASK_STK[MOTOR_PROC_STK_SIZE];          //���������ջ�ռ�	
extern void motor_proc_task(void *pdata);                //�������ӿ�

/***** motor���ݴ������� *****/
#define FAN_PROC_TASK_PRIO     10                       //�����������ȼ�
#define FAN_PROC_STK_SIZE      256                     //���������ջ��С
OS_STK FAN_PROC_TASK_STK[FAN_PROC_STK_SIZE];          //���������ջ�ռ�	
extern void fan_proc_task(void *pdata);                //�������ӿ�

/**** ����������Ϣ���� ****/
#define MaxBluetoothTaskMsgQTab        	8
void *BluetoothTaskMsgQTab        		[MaxBluetoothTaskMsgQTab];
OS_EVENT    *BluetoothMsgQ;

OS_FLAG_GRP *TaskCheckOSFlag;
/****************************************************************************************
 ������
****************************************************************************************/

int main(void)
{
	delay_init();	     //��ʱ��ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�

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
 	OSTaskCreate(start_task,(void *)0,(OS_STK *)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO );//������ʼ����
	OSStart();
}



/****************************************************************************************
 ��������
****************************************************************************************/
void start_task(void *pdata)
{
    u8          err          = 0;
    u16         TaskCheckCnt = 0;
    OS_CPU_SR   cpu_sr       = 0;
	OS_FLAGS    run_flag;
	pdata = pdata; 
	OSStatInit();					//��ʼ��ͳ������.�������ʱ1��������	
 	OS_ENTER_CRITICAL();			//�����ٽ���(�޷����жϴ��)    

	BluetoothMsgQ      = OSQCreate(BluetoothTaskMsgQTab,      MaxBluetoothTaskMsgQTab);

 	OSTaskCreate(bluetooth_proc_task,   (void *)0,(OS_STK*)&BLUETOOTH_PROC_TASK_STK[BLUETOOTH_PROC_STK_SIZE-1],      BLUETOOTH_PROC_TASK_PRIO);
 	OSTaskCreate(bluetooth_receive_task,(void *)0,(OS_STK*)&BLUETOOTH_RECEIVE_TASK_STK[BLUETOOTH_RECEIVE_STK_SIZE-1],BLUETOOTH_RECEIVE_TASK_PRIO);

 	OSTaskCreate(fan_proc_task,(void *)0,(OS_STK*)&FAN_PROC_TASK_STK[FAN_PROC_STK_SIZE-1],FAN_PROC_TASK_PRIO);
 	OSTaskCreate(motor_proc_task,(void *)0,(OS_STK*)&MOTOR_PROC_TASK_STK[MOTOR_PROC_STK_SIZE-1],MOTOR_PROC_TASK_PRIO);

    //finsh_system_init();

 	TaskCheckOSFlag = OSFlagCreate(0,&err);

	OS_EXIT_CRITICAL();				//�˳��ٽ���(���Ա��жϴ��)

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
				NVIC_SystemReset();// ��λ
			}
		}
		else
			TaskCheckCnt = 0;
		OSTimeDly(100);
 	}
}

