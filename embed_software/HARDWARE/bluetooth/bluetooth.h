#ifndef __BLUETOOTH_DEF
#define __BLUETOOTH_DEF
#include "sys.h"
#include "fan.h"

typedef struct __GROUP_INFO
{
	u8		motor_angle;		//舵机角度
	u16		fan_time;			//风扇转动时间
	u8		fan_level;			//风扇转动等级
}GROUP_INFO;

typedef struct __PARAM_INFO
{
	GROUP_INFO	group1;			//舵机1
	GROUP_INFO	group2;			//舵机2
	GROUP_INFO	group3;			//舵机3

	u8			alarm_info;		//闹铃日期
	u8			alarm_hour;		//小时
	u8			alarm_min;		//分钟
//	u8          crc;			//校验
}PARAM_INFO;
extern PARAM_INFO param;
extern u8 flush_motor_flag;
extern u8 flush_fan_flag;
extern u8 flush_stop_flag;
extern u8 pwr_flag;

extern int rfid_send_cmd_to_bluetooth_task(u32 Type,u32 len,u8* p_data);
#endif  //__BLUETOOTH_DEF
