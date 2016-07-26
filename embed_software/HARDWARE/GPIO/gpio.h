#ifndef __GPIO_DEF_H
#define __GPIO_DEF_H

#define FAN_SET(x)    GPIOA->ODR=(GPIOA->ODR&~(1<<11))|(x ? (1<<11):0) 

extern void bluetooth_gpio_init(void);
extern void pwm_gpio_init(void);
extern void rfid_gpio_init(void);
extern void led_gpio_init(void);
extern void Charge_Gpio_Init(void);
#endif

