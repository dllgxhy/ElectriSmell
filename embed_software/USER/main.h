#ifndef __MAIN_TASK_H
#define __MAIN_TASK_H

#define OSFLAG_BLUETOOTH_PROC_TASK      (1u<<0)
#define OSFLAG_BLUETOOTH_RECEIVE_TASK   (1u<<1)
#define OSFLAG_RFID_PROC_TASK           (1u<<2)
#define OSFLAG_MOTOR_PROC_TASK			(1u<<3)
#define OSFLAG_FAN_PROC_TASK			(1u<<4)

#define MSG_ID_BLUERECTASK      0x22
#define MSG_ID_BLUEPROCTASK     0x33
#define MSG_ID_RFIDTASK        	0x44 

typedef struct {
    int 	source;        	// message come from ....
    int 	type;			// message id
    int 	para1;			// message param 1
    int 	para2;			// message param 2
    void   *DataBuf;		// message data
}MSGSTRUCT,*pMSGSTRUCT;

/****************************************************************
蓝牙发送任务消息队列
****************************************************************/
#define MaxBluetoothTaskMsgQTab        	8
extern OS_EVENT    *BluetoothMsgQ;

/****************************************************************
消息队列ID
****************************************************************/
#define RFID_SEND_CARD_MSG	0x01	//rfid任务发送给蓝牙发送任务的ID

/****************************************************************
	time value define
****************************************************************/
#define T_5MS		1

#define T_10MS		1
#define T_50MS		(T_10MS*5)
#define T_100MS		(T_10MS*10)
#define T_200MS		(T_10MS*20)
#define T_250MS		(T_10MS*25)
#define T_300MS		(T_10MS*30)
#define	T_500MS	    (T_10MS*50)
#define	T_600MS	    (T_100MS*6)
#define	T_1500MS	(T_500MS*3)

#define	T_1S		100//200
#define	T_2S		(T_1S*2)
#define	T_3S		(T_1S*3)
#define	T_4S		(T_1S*4)
#define	T_5S		(T_1S*5)
#define	T_6S		(T_1S*6)
#define	T_9S		(T_1S*9)
#define	T_10S		(T_1S*10)
#define	T_20S		(T_1S*20)
#define	T_25S		(T_1S*25)
#define	T_30S		(T_1S*30)
#define	T_40S		(T_1S*40)

#define T_1MIN		(T_1S*60)
#define T_2MIN		(T_1MIN*2)
#define T_3MIN		(T_1MIN*3)
#define T_4MIN		(T_1MIN*4)
#define T_5MIN		(T_1MIN*5)
#define T_10MIN		(T_1MIN*10)
#define T_20MIN		(T_1MIN*20)
#define T_40MIN		(T_1MIN*40)
#define T_1HOUR		(T_1MIN*60)
#define T_1DAY		(24*T_1HOUR)

#endif

