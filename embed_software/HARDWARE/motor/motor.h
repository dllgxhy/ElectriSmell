#ifndef _MOTOR_H_
#define _MOTOR_H_


#define RESET_MOTOR_POSITION
#define DEFAULT_MOTOR_RESET_ANGLE 0x00 //默认状态下，舵机在90度停


typedef enum{
	MOTOR_ID1 = 0x01,
	MOTOR_ID2,
	MOTOR_ID3,
	MOTOR_DEFAULT,
}MOTOR_ID;
extern void Motor_Init(void);
extern void Reset_Motor_Position(void);

extern void Motor_change_angle_to(MOTOR_ID id,u16 angle);

#endif

