#ifndef _FAN_H_
#define _FAN_H_

extern void Fan_Init(void);

typedef enum{
	FAN_LEVEL1 = 0x01,
	FAN_LEVEL2,
	FAN_LEVEL3,
	FAN_LEVEL_DEFAULT,
}FAN_LEVEL;

#endif

