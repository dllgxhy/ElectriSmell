/*
  retarge.c
  
  IAR LIB设置Normal
  重写int putchar(int ch)
  
  IAR LIB设置Full
  重写int putchar(int ch) 或者 int fputc(int ch, FILE *f) 都可以。
  
  都是调用putchar函数。
  只是设置Full的时候，putchar函数调用了又调用fputc函数，所以重写fputc也可以。
  putchar(ch)相当于与fputc(ch,stdout)
  设置NORMAL的时候，不支持文件指针，也没有定义stdout，不能调用fputc(ch,stdout)。
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "sys.h"
#include "finsh.h"
#include "debug.h"

unsigned int gDebugFlag = NORM_DBG;
/*==================================================================
*Description ：打印开关调试函数，根据打印菜单进行操作，打开和关闭相
			   应模块的打印
*Return Value：
*Input		 : 
*Output 	 :
*Globle Var  ：
==================================================================*/
void dbgset(unsigned int choice,unsigned char flag)
{
    switch(choice)
    {
    case 0:
        InfoPrintf(" 1: NORM_DBG 系统共享内存调试信息\r\n");
        InfoPrintf(" 2: OTHER_DBG 其他类型的调试信息\r\n");
        break;
    case ON_OFF_NORM_DBG:
        if(!flag)
        	gDebugFlag &= ~NORM_DBG;
        else
        	gDebugFlag |= NORM_DBG;					
        break;
    case ON_OFF_OTHER_DBG:
        if(!flag)
            gDebugFlag &= ~OTHER_DBG;
        else
            gDebugFlag |= OTHER_DBG;					
        break;
    case ON_ALL_DEBUG:
        gDebugFlag = 0xFFFFFFFF;
        break;
    case OFF_ALL_DEBUG:
        gDebugFlag = 0;
        break;	
    default:
        InfoPrintf("\nInvalid input,press 0 for help.%d",choice);				
        break;		
    }
    InfoPrintf("\n");
}
FINSH_FUNCTION_EXPORT(dbgset,setdbg on/off(0~31))

