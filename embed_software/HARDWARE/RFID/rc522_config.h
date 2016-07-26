 /* 注： 通过此文件可以修改RFID的引脚连线    */

#define u8  unsigned char
#define u16  unsigned int
#define RST  (1<<11)				//PA11: RST
#define TCS  (1<<4)					//PA4:  CS

//port 0
#define  CLR_RC522RST  GPIOA->ODR=(GPIOA->ODR&~RST)|(0 ? RST:0)
#define  SET_RC522RST  GPIOA->ODR=(GPIOA->ODR&~RST)|(1 ? RST:0)

#define DOUT GPIOA->IDR&(1<<0X06)	//PA6:MISO
			 
#define TDIN  (1<<0X07)				//PA7:MOSI
#define TCLK  (1<<0X05) 			//PA5:CLK
#define TDIN_SET(x) GPIOA->ODR=(GPIOA->ODR&~TDIN)|(x ? TDIN:0)
#define TCLK_SET(x) GPIOA->ODR=(GPIOA->ODR&~TCLK)|(x ? TCLK:0)													    
#define TCS_SET(x)  GPIOA->ODR=(GPIOA->ODR&~TCS)|(x ? TCS:0) 

//port 1
#define RST1  (1<<15)				//PB15: RST
#define TCS1  (1<<14)				//PB14: CS
#define CLR1_RC522RST  GPIOB->ODR=(GPIOB->ODR&~RST1)|(0 ? RST1:0)
#define SET1_RC522RST  GPIOB->ODR=(GPIOB->ODR&~RST1)|(1 ? RST1:0)
#define TCS1_SET(x)    GPIOB->ODR=(GPIOB->ODR&~TCS1)|(x ? TCS1:0) 

