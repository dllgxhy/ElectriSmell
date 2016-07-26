#ifndef __BLUETOOTH_DEF
#define __BLUETOOTH_DEF
#include "sys.h"
#include "fan.h"

typedef struct __GROUP_INFO
{
	u8		motor_angle;		//����Ƕ�
	u16		fan_time;			//����ת��ʱ��
	u8		fan_level;			//����ת���ȼ�
}GROUP_INFO;

typedef struct __PARAM_INFO
{
	GROUP_INFO	group1;			//���1
	GROUP_INFO	group2;			//���2
	GROUP_INFO	group3;			//���3

	u8			alarm_info;		//��������
	u8			alarm_hour;		//Сʱ
	u8			alarm_min;		//����
//	u8          crc;			//У��
}PARAM_INFO;
extern PARAM_INFO param;
extern u8 flush_motor_flag;
extern u8 flush_fan_flag;
extern u8 flush_stop_flag;
extern u8 pwr_flag;

extern int rfid_send_cmd_to_bluetooth_task(u32 Type,u32 len,u8* p_data);
#endif  //__BLUETOOTH_DEF
