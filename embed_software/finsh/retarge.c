/*
  retarge.c
  
  IAR LIB����Normal
  ��дint putchar(int ch)
  
  IAR LIB����Full
  ��дint putchar(int ch) ���� int fputc(int ch, FILE *f) �����ԡ�
  
  ���ǵ���putchar������
  ֻ������Full��ʱ��putchar�����������ֵ���fputc������������дfputcҲ���ԡ�
  putchar(ch)�൱����fputc(ch,stdout)
  ����NORMAL��ʱ�򣬲�֧���ļ�ָ�룬Ҳû�ж���stdout�����ܵ���fputc(ch,stdout)��
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "sys.h"
#include "finsh.h"
#include "debug.h"

unsigned int gDebugFlag = NORM_DBG;
/*==================================================================
*Description ����ӡ���ص��Ժ��������ݴ�ӡ�˵����в������򿪺͹ر���
			   Ӧģ��Ĵ�ӡ
*Return Value��
*Input		 : 
*Output 	 :
*Globle Var  ��
==================================================================*/
void dbgset(unsigned int choice,unsigned char flag)
{
    switch(choice)
    {
    case 0:
        InfoPrintf(" 1: NORM_DBG ϵͳ�����ڴ������Ϣ\r\n");
        InfoPrintf(" 2: OTHER_DBG �������͵ĵ�����Ϣ\r\n");
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

