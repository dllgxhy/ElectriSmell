#include "includes.h"					//ucos ʹ��
#include "stm32f10x.h"
#include "usart.h"
#include "main.h"
#include "debug.h"
#include "bluetooth.h"
#include "timer.h"
#include "rtc.h"
#include "wkup.h"
#include "flash.h"

extern OS_FLAG_GRP *TaskCheckOSFlag;
uint8_t pwr_flag = 0x00;  //��¼��ǰ�ĵ�Դ״̬
PARAM_INFO param = {0};
u8 flush_motor_flag = 0;
u8 flush_fan_flag   = 0;
u8 flush_stop_flag  = 0;

#define BLUETOOTH_BUF_SIZE      512
static u8  bluetooth_rec_buf[BLUETOOTH_BUF_SIZE] = {0};
static u16 bluetooth_len = 0;

/****************************************************************************************
 �������ú���
****************************************************************************************/
void set_alarm_time(void)
{
	_calendar_obj rtc;
	//�ж��Ƿ�����������
	if( !(param.alarm_info & 0xfe) )
		return;
	
	//��ȡ��ǰ��ʱ��
	RTC_Get();
	ALARM_Set(calendar.w_year,calendar.w_month,calendar.w_date,param.alarm_hour,param.alarm_min,0);
//	if( param.alarm_info & (1 <<(calendar.week+1)))
//		ALARM_Set(calendar.w_year,calendar.w_month,calendar.w_date,param.alarm_hour,param.alarm_min,0);
//	else
//	{	//��ȡ��һ���ʱ��
//		RTC_GetNextDay(calendar.w_year,calendar.w_month,calendar.w_date,&rtc);
//		//������һ��Ļ���ʱ��
//		ALARM_Set(rtc.w_year,rtc.w_month,rtc.w_date,param.alarm_hour,param.alarm_min,0);
//	}
}

/****************************************************************************************
 �������ݽ��մ�����
****************************************************************************************/
static u32 analyze_bluetooth_data(u8 *p_data,u16 size)
{
    debug_printf(NORM_DBG,p_data,size);
	switch( p_data[0] )
	{
	case 0x01:	//�������Ȼ��߶��
		if( flush_stop_flag )
		{
			flush_stop_flag = 0x02;
			while(flush_stop_flag)
				OSTimeDly(5);
		}
		param.group1.motor_angle = p_data[1];
		param.group1.fan_time    = (p_data[2]<<8) + p_data[3];
		param.group1.fan_level   = p_data[4];

		param.group2.motor_angle = p_data[5];
		param.group2.fan_time    = (p_data[6]<<8) + p_data[7];
		param.group2.fan_level   = p_data[8];

		param.group3.motor_angle = p_data[9];
		param.group3.fan_time    = (p_data[10]<<8) + p_data[11];
		param.group3.fan_level   = p_data[12];
		flush_motor_flag = 1;
		size = 13;
		break;
	case 0x02:	//��������
		param.alarm_info =  p_data[1]; //����������
		param.alarm_hour =  p_data[2];	//Сʱ����
		param.alarm_min  =  p_data[3];	//��������

		param.group1.motor_angle = p_data[4];
		param.group1.fan_time    = (p_data[5]<<8) + p_data[6];
		param.group1.fan_level = p_data[7];

		param.group2.motor_angle = p_data[8];
		param.group2.fan_time    = (p_data[9]<<8) + p_data[10];
		param.group2.fan_level = p_data[11];

		param.group3.motor_angle = p_data[12];
		param.group3.fan_time    = (p_data[13]<<8) + p_data[14];
		param.group3.fan_level = p_data[15];
		write_param_info(&param);
		set_alarm_time();					//��������ʱ��
		size = 16;
		break;
	case 0x03:	//��ʱ,ʱ��ͬ��
		RTC_Set( (p_data[1]<<8) + p_data[2],	//��
				p_data[3],						//��
				p_data[4],						//��
				p_data[5],						//ʱ
				p_data[6],						//��
				p_data[7]);						//��
		size = 8;
		break;
	}
    return size;
}

#define MAX_DATA_LEN 20  //ÿ��
/****************************************************************************************
 �������ݽ��մ�����
****************************************************************************************/
static void receive_data_proc(void)
{
    u8  buf[100] = {0};
    u8  len = 0;
	u8  flag = 0;
    u32 i = 0;
    u32 idx = 0;
    u32 size = 0;
    len = UART_Read(BLUE_TOOTH_DEV,buf,MAX_DATA_LEN);
    if( len )
    {
		//InfoPrintf("rec blue tooth data: %s\r\n",buf);
        if( (bluetooth_len + len) < BLUETOOTH_BUF_SIZE )
        {
            memcpy(&bluetooth_rec_buf[bluetooth_len],buf,len);
            bluetooth_len += len;
        }

        for( i = 0 ; i < bluetooth_len ; i++ )
        {
        	if( bluetooth_rec_buf[i] == 0xfe && bluetooth_rec_buf[i+1] == 0xfd)
    		{	//��ͷ
				i++;
    			idx  = 0;
    			flag = 1;
    		}
			else if( bluetooth_rec_buf[i] == 0xfe && bluetooth_rec_buf[i+1] == 0xfb)
			{	//��β
				flag       = 0;
                size      += analyze_bluetooth_data(buf,idx);
                size      += 4;
                idx        = 0;
                i++;
			}
            else
            {
            	if( flag == 1 )
	                buf[idx++] = bluetooth_rec_buf[i];
            }
        }
    }
    if( size )
    {
        if( size >= bluetooth_len )
        {
            memset(bluetooth_rec_buf,0,BLUETOOTH_BUF_SIZE );
            bluetooth_len = 0;
        }
        else
        {
            memcpy(bluetooth_rec_buf,&bluetooth_rec_buf[size],bluetooth_len - size);
            bluetooth_len -= size;
        }
    }
}

/****************************************************************************************
 ������ʼ��
****************************************************************************************/
static void bluetooth_init()
{
    /* ��ʼ������ATָ�� */
    BluetoothPrintf("AT\r\n");
    BluetoothPrintf("AT+ADDR\r\n");
    BluetoothPrintf("AT+TYPE\r\n");
    BluetoothPrintf("AT+NAME\r\n");
    BluetoothPrintf("AT+IMME\r\n");
    BluetoothPrintf("AT+ADVI\r\n");
    BluetoothPrintf("AT+VERSION\r\n");
    BluetoothPrintf("AT+LADDR\r\n");
    BluetoothPrintf("AT+PIN\r\n");
    BluetoothPrintf("AT+TYPE\r\n");
    BluetoothPrintf("AT+ROLE\r\n");
    BluetoothPrintf("AT+UUID\r\n");
    BluetoothPrintf("AT+CHAR\r\n");
//    BluetoothPrintf("AT+HELP\r\n");
}

/****************************************************************************************
 ������Ϣ�������
****************************************************************************************/
static void bluetooth_proc_handle(void)
{
	u8			err 		 =	0;
	MSGSTRUCT  *Msg 		 = NULL;
	
	Msg = (MSGSTRUCT  *)OSQAccept(BluetoothMsgQ,&err);	//����Ϣ�����ж�ȡ��Ϣ
	if( Msg != NULL )
	{
		switch( Msg->source )
		{
		case MSG_ID_RFIDTASK:		//rfid������Ϣ
			switch( Msg->type )
			{
			case RFID_SEND_CARD_MSG:
				BluetoothPrintf("%s",Msg->DataBuf);	//rfid�����ṩ���ݣ�ֱ�ӷ���
				DebugPrintf(OTHER_DBG,"send bluetooth data: %s\r\n",Msg->DataBuf);
				break;
			default:
				InfoPrintf("found unknow msg from rfid task: %d\r\n",Msg->type);
				break;
			}
			break;
		case MSG_ID_BLUERECTASK:	//bluetooth������������Ϣ
			InfoPrintf("found unknow msg from bluetooth receive task\r\n");
			break;
		default:
			InfoPrintf("blooth proc task found unknow msg: %d\r\n",Msg->source);
			break;
		}
	}
}

/****************************************************************************
* ���ƣ�rfid_send_cmd_to_bluetooth_task()
* ���ܣ�rfid���������ݵ�������������
* sour   -> ��Ϣ�����ĸ�����
* type   -> msg id
* para1  -> param 1
* para2  -> param 2
* p_data -> data buf
****************************************************************************/
int rfid_send_cmd_to_bluetooth_task(u32 Type,u32 len,u8* p_data)
{
    static MSGSTRUCT CtrlMBuf[MaxBluetoothTaskMsgQTab + 1]      	= {0};
    static u8     	 Ctr_Index      =  0;
    static u8     	 data_buf[(MaxBluetoothTaskMsgQTab + 1) * 32]	= {0};
    u8            	 err            =  0;
    u32	             StartTime      =  0;

	if( len && p_data)
	{
		memcpy(&data_buf[Ctr_Index * 32], p_data,len);
		CtrlMBuf[Ctr_Index].DataBuf = &data_buf[Ctr_Index * 32];
	}
	else
		CtrlMBuf[Ctr_Index].DataBuf = NULL;

	CtrlMBuf[Ctr_Index].source  = MSG_ID_RFIDTASK;
	CtrlMBuf[Ctr_Index].type    = Type;
	CtrlMBuf[Ctr_Index].para1   = len; 
	CtrlMBuf[Ctr_Index].para2   = 0;

	StartTime = OSTimeGet();
	while(1)  
    {   		 
		err = OSQPost(BluetoothMsgQ,(void *)(&CtrlMBuf[Ctr_Index]));   
		if ( err != OS_ERR_NONE ) 
		 {
		 	if( (OSTimeGet() - StartTime) >= T_5S )
				return -1;	
			OSTimeDly(1);			
	 		continue; 
		} 
		else
		{ 
			Ctr_Index = (Ctr_Index+1) % ( MaxBluetoothTaskMsgQTab + 1) ;
			return 0;
		}
	}
}

/****************************************************************************************
 �������ݽ�������
****************************************************************************************/
void bluetooth_receive_task(void *pdata)
{
    u8  err;
    
    pdata = pdata;
    while(1)
    {
        receive_data_proc();

        OSTimeDly(5);
		OSFlagPost(TaskCheckOSFlag,OSFLAG_BLUETOOTH_RECEIVE_TASK,OS_FLAG_SET,&err);
    }
}

#define  EQUIPMENT_OPEN   0x01
#define  EQUIPMENT_CLOSE  0x00
uint8_t EQUIPMENT_STATUS = EQUIPMENT_CLOSE;
/****************************************************************************************
 ����/���Ѽ��������
****************************************************************************************/
void check_wkup_key(void)
{
	if( pwr_flag == 0 )
	{
		if( PAin(0) == 1 )
			pwr_flag = 1;
	}
	else
	{
		if(PAin(0) == 0)
		{
			pwr_flag = 0;
			if (EQUIPMENT_STATUS == EQUIPMENT_CLOSE)
			{
				EQUIPMENT_STATUS = EQUIPMENT_OPEN;
			}
			else
			{
				EQUIPMENT_STATUS = EQUIPMENT_CLOSE;
			}
		}
	}
} 

#define  CHARGEING    0x01
#define  CHARGE_FULL  0x00
uint8_t CHARGE_FLAG = CHARGE_FULL;
void check_charge_key(void)
{
	if(PAin(12) == 0)   //��������
	{
		CHARGE_FLAG = CHARGEING;	
	}
	else	   			//������
	{		
		CHARGE_FLAG = CHARGE_FULL;		
	}	
}

void LED_work_status(void)
{
	check_wkup_key();
	check_charge_key();
	if((CHARGE_FLAG == CHARGE_FULL)&(EQUIPMENT_STATUS == EQUIPMENT_OPEN))
	{
		PDout(13) = 0x01;
		PCout(2) = 0x01;
		PCout(3) = 0x01;
		PCout(4) = 0x01;
		PCout(5) = 0x01;
	}
	else if(CHARGE_FLAG == CHARGEING)
	{
		PDout(13) = ~PDout(13);
		PCout(2) = ~PCout(2);
		PCout(3) = ~PCout(3);
		PCout(4) = ~PCout(4);
		OSTimeDly(200);	
	}
	else if(EQUIPMENT_STATUS == EQUIPMENT_CLOSE)
	{
		PDout(13) = 0X00;
		PCout(2) = 0x00;
		PCout(3) = 0x00;
		PCout(4) = 0x00;
		PCout(5) = 0x00;	
	}
	else
	{
	} 	
}
/****************************************************************************************
 �������ݴ�������
****************************************************************************************/
void bluetooth_proc_task(void *pdata)
{
	u8  err;
	pdata = pdata;

    bluetooth_init();
    while(1)
    {
    	bluetooth_proc_handle();	//����������Ϣ������

<<<<<<< HEAD
=======
//		check_wkup_key();
//		check_charge_key();
>>>>>>> 84223f3ea0a6e5eeb048a3177ac34ec8b689dfdc
		LED_work_status();
		if(RTCAlarm_Flag == 0x01)
		{
			read_param_info(&param);
			flush_motor_flag = 1;
			RTCAlarm_Flag = 0x00;
		}
		
        OSTimeDly(5);
		OSFlagPost(TaskCheckOSFlag,OSFLAG_BLUETOOTH_PROC_TASK,OS_FLAG_SET,&err);
    }
}

