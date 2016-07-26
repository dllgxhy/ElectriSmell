/*
 * File      : shell.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-04-30     Bernard      the first verion for FinSH
 * 2006-05-08     Bernard      change finsh thread stack to 2048
 * 2006-06-03     Bernard      add support for skyeye
 * 2006-09-24     Bernard      remove the code related with hardware
 * 2010-01-18     Bernard      fix down then up key bug.
 * 2010-03-19     Bernard      fix backspace issue and fix device read in shell.
 * 2010-04-01     Bernard      add prompt output when start and remove the empty history
 * 2011-02-23     Bernard      fix variable section end issue of finsh shell
 *                             initialization when use GNU GCC compiler.
 */
/*
*  使用说明： 根据需要修改
*
*  keil：--keep __fsym_* --keep __vsym_*
*
*/



//#include <rtthread.h>
//#include <rthw.h>

#include "finsh.h"
#include "shell.h"
#include "finsh_heap.h"
#include "ucos_ii.h"
#include "sys.h"
#include "usart.h"	

#define FINSH_CFG_TASK_STK_SIZE 512
#define FINSH_TASK_PRIO   4//(OS_LOWEST_PRIO - 7)


static  OS_STK       FINSH_Task_Stk[FINSH_CFG_TASK_STK_SIZE];
//OS_EVENT *Q_finsh_rx = NULL;
extern unsigned  char  Get_TestPin_State(void);
/* finsh thread */
//static struct rt_thread finsh_thread;
ALIGN(RT_ALIGN_SIZE)
//static char finsh_thread_stack[FINSH_THREAD_STACK_SIZE];
struct finsh_shell* gtShell;

#if !defined (RT_USING_NEWLIB) && !defined (RT_USING_MINILIBC)
int strcmp (const char *s1, const char *s2)
{
	while (*s1 && *s1 == *s2) s1++, s2++;

	return (*s1 - *s2);
}

#ifdef RT_USING_HEAP

void *rt_memcpy(void *dst, const void *src, rt_ubase_t count)
{
	char *tmp = (char *)dst, *s = (char *)src;

	while (count--)
		*tmp++ = *s++;

	return dst;

}

char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *tmp = (char *)finsh_heap_allocate(len);

	if(!tmp) return NULL;

	rt_memcpy(tmp, s, len);
	return tmp;
}
#endif

#if !defined(__CC_ARM) && !defined(__IAR_SYSTEMS_ICC__)
int isalpha( int ch )
{
	return (unsigned int)((ch | 0x20) - 'a') < 26u;
}

int atoi(const char* s)
{
	long int v=0;
	int sign=1;
	while ( *s == ' '  ||  (unsigned int)(*s - 9) < 5u) s++;

	switch (*s)
	{
	case '-': sign=-1;
	case '+': ++s;
	}

	while ((unsigned int) (*s - '0') < 10u)
	{
		v=v*10+*s-'0'; ++s;
	}

	return sign==-1?-v:v;
}

int isprint(unsigned char ch)
{
    return (unsigned int)(ch - ' ') < 127u - ' ';
}
#endif
#endif

#if defined(RT_USING_DFS) && defined(DFS_USING_WORKDIR)
#include <dfs_posix.h>
const char* finsh_get_prompt()
{
	#define _PROMPT "finsh "
	static char finsh_prompt[RT_CONSOLEBUF_SIZE + 1] = {_PROMPT};
	
	/* get current working directory */
	getcwd(&finsh_prompt[6], RT_CONSOLEBUF_SIZE - 8);
	strcat(finsh_prompt, ">");

	return finsh_prompt;
}
#endif
#if 0
static rt_err_t finsh_rx_ind(rt_device_t dev, rt_size_t size)
{
	RT_ASSERT(gtShell != RT_NULL);

	/* release semaphore to let finsh thread rx data */
	rt_sem_release(&gtShell->rx_sem); //liao

	return RT_EOK;
}

/**
 * @ingroup finsh
 *
 * This function sets the input device of finsh shell.
 *
 * @param device_name the name of new input device.
 */
void finsh_set_device(const char* device_name)
{
	rt_device_t dev = RT_NULL;

	RT_ASSERT(gtShell != RT_NULL);
	dev = rt_device_find(device_name);
	if (dev != RT_NULL && rt_device_open(dev, RT_DEVICE_OFLAG_RDWR) == RT_EOK)
	{
		if (gtShell->device != RT_NULL)
		{
			/* close old finsh device */
			rt_device_close(gtShell->device);
		}

		gtShell->device = dev;
		rt_device_set_rx_indicate(dev, finsh_rx_ind);
	}
	else
	{
		sh_printf("finsh: can not find device:%s\n", device_name);
	}
}

/**
 * @ingroup finsh
 *
 * This function returns current finsh shell input device.
 *
 * @return the finsh shell input device name is returned.
 */
const char* finsh_get_device()
{
	RT_ASSERT(gtShell != RT_NULL);
	return gtShell->device->parent.name;
}
#endif
/**
 * @ingroup finsh
 *
 * This function set the echo mode of finsh shell.
 *
 * FINSH_OPTION_ECHO=0x01 is echo mode, other values are none-echo mode.
 *
 * @param echo the echo mode
 */
void finsh_set_echo(rt_uint32_t echo)
{
	RT_ASSERT(gtShell != RT_NULL);
	gtShell->echo_mode = echo;
}

/**
 * @ingroup finsh
 *
 * This function gets the echo mode of finsh shell.
 *
 * @return the echo mode
 */
rt_uint32_t finsh_get_echo()
{
	RT_ASSERT(gtShell != RT_NULL);

	return gtShell->echo_mode;
}

void finsh_auto_complete(char* prefix)
{
	extern void list_prefix(char* prefix);

	sh_printf("\r\n");
	list_prefix(prefix);
	sh_printf("%s%s", FINSH_PROMPT, prefix);
}

void finsh_run_line(struct finsh_parser* parser, const char *line)
{
	const char* err_str;

	sh_printf("\r\n");
	finsh_parser_run(parser, (unsigned char*)line);

	/* compile node root */
	if (finsh_errno() == 0)
	{
		finsh_compiler_run(parser->root);
	}
	else
	{
		err_str = finsh_error_string(finsh_errno());
		sh_printf("%s\r\n", err_str);
	}

	/* run virtual machine */
	if (finsh_errno() == 0)
	{
		char ch;
		finsh_vm_run();

		ch = (unsigned char)finsh_stack_bottom();
		if (ch > 0x20 && ch < 0x7e)
		{
			sh_printf("\t'%c', %d, 0x%08x\r\n",
				(unsigned char)finsh_stack_bottom(),
				(unsigned int)finsh_stack_bottom(),
				(unsigned int)finsh_stack_bottom());
		}
		else
		{
            sh_printf("\t%d, 0x%08x\r\n",
                (unsigned int)finsh_stack_bottom(),
                (unsigned int)finsh_stack_bottom());
		}
	}

    finsh_flush(parser);
}

#ifdef FINSH_USING_HISTORY
rt_bool_t finsh_handle_history(struct finsh_shell* shell, char ch)
{
	/*
	 * handle up and down key
	 * up key  : 0x1b 0x5b 0x41
	 * down key: 0x1b 0x5b 0x42
	 */
	if (ch == 0x1b)
	{
		shell->stat = WAIT_SPEC_KEY;
		return RT_TRUE;
	}

	if ((shell->stat == WAIT_SPEC_KEY))
	{
		if (ch == 0x5b)
		{
			shell->stat = WAIT_FUNC_KEY;
			return RT_TRUE;
		}

		shell->stat = WAIT_NORMAL;
		return RT_FALSE;
	}

	if (shell->stat == WAIT_FUNC_KEY)
	{
		shell->stat = WAIT_NORMAL;

		if (ch == 0x41) /* up key */
		{
			/* prev history */
			if (shell->current_history > 0)
				shell->current_history --;
			else
			{
				shell->current_history = 0;
				return RT_TRUE;
			}

			/* copy the history command */
			memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
				FINSH_CMD_SIZE);
			shell->line_position = strlen(shell->line);
			shell->use_history = 1;
		}
		else if (ch == 0x42) /* down key */
		{
			/* next history */
			if (shell->current_history < shell->history_count - 1)
				shell->current_history ++;
			else
			{
				/* set to the end of history */
				if (shell->history_count != 0)
				{
					shell->current_history = shell->history_count - 1;
				}
				else return RT_TRUE;
			}

			memcpy(shell->line, &shell->cmd_history[shell->current_history][0],
				FINSH_CMD_SIZE);
			shell->line_position = strlen(shell->line);
			shell->use_history = 1;
		}

		if (shell->use_history)
		{
			sh_printf("\033[2K\r");
			sh_printf("%s%s", FINSH_PROMPT, shell->line);
			return RT_TRUE;;
		}
	}

	return RT_FALSE;
}

void finsh_push_history(struct finsh_shell* shell)
{
	if ((shell->use_history == 0) && (shell->line_position != 0))
	{
		/* push history */
		if (shell->history_count >= FINSH_HISTORY_LINES)
		{
			/* move history */
			int index;
			for (index = 0; index < FINSH_HISTORY_LINES - 1; index ++)
			{
				memcpy(&shell->cmd_history[index][0],
					&shell->cmd_history[index + 1][0], FINSH_CMD_SIZE);
			}
			memset(&shell->cmd_history[index][0], 0, FINSH_CMD_SIZE);
			memcpy(&shell->cmd_history[index][0], shell->line, shell->line_position);

			/* it's the maximum history */
			shell->history_count = FINSH_HISTORY_LINES;
		}
		else
		{
			memset(&shell->cmd_history[shell->history_count][0], 0, FINSH_CMD_SIZE);
			memcpy(&shell->cmd_history[shell->history_count][0], shell->line, shell->line_position);

			/* increase count and set current history position */
			shell->history_count ++;
		}
	}
	shell->current_history = shell->history_count;
}
#endif

#ifndef RT_USING_HEAP
struct finsh_shell _shell;
#endif
void finsh_thread_entry(void* parameter)
{
    char ch;
    INT8U err;

	/* normal is echo mode */
	gtShell->echo_mode = 1;

   finsh_init(&gtShell->parser);   
	sh_printf(FINSH_PROMPT);

	while (1)
	{
		/* wait receive */
		//if (rt_sem_take(&gtShell->rx_sem, RT_WAITING_FOREVER) != RT_EOK) continue;
		
		/* read one character from device */
		//while (rt_device_read(gtShell->device, 0, &ch, 1) == 1)

    	if(UART_Read(DEBUG_DEV, (unsigned char*)&ch,1) == 0) 
		{
			OSTimeDly(20);
			continue;//查询法
        }
        
       // pChar = OSQPend(Q_finsh_rx,0,&err);//wait forever
       // ch = *pChar; 
		{
			/* handle history key */
			#ifdef FINSH_USING_HISTORY
			if (finsh_handle_history(gtShell, ch) == RT_TRUE) continue;
			#endif

			/* handle CR key */
			if (ch == '\r')
			{
				char next = '\n';
                
                
				//if (rt_device_read(gtShell->device, 0, &next, 1) == 1)
			   if(UART_Read(DEBUG_DEV, (unsigned char*)&ch,1) == 0) 
//                pChar = OSQPend(Q_finsh_rx,100,&err);//wait 100 ticks
               if(err == OS_ERR_NONE)
               { 
                    //ch = *pChar;
				    ch = next;
               }
			  else ch = '\r';

				
				
        
			}
			/* handle tab key */
			else if (ch == '\t')
			{
				/* auto complete */
				finsh_auto_complete(&gtShell->line[0]);
				/* re-calculate position */
				gtShell->line_position = strlen(gtShell->line);
				continue;
			}
			/* handle backspace key */
			else if (ch == 0x7f || ch == 0x08)
			{
				if (gtShell->line_position != 0)
				{
					sh_printf("%c %c", ch, ch);
				}
				if (gtShell->line_position <= 0) 
					gtShell->line_position = 0;
				else 
					gtShell->line_position --;
				gtShell->line[gtShell->line_position] = 0;
				continue;
			}

			/* handle end of line, break */
			if (ch == '\r' || ch == '\n')
			{
				/* change to ';' and break */
				gtShell->line[gtShell->line_position] = ';';

				#ifdef FINSH_USING_HISTORY
				finsh_push_history(gtShell);
				#endif

				if (gtShell->line_position != 0) 
					finsh_run_line(&gtShell->parser, gtShell->line);
				else
					sh_printf("\r\n");

                sh_printf(FINSH_PROMPT);
				memset(gtShell->line, 0, sizeof(gtShell->line));
				gtShell->line_position = 0;
        
				//break;
                continue;
			}

			/* it's a large line, discard it */
			if (gtShell->line_position >= FINSH_CMD_SIZE) 
			{
				sh_printf("cmd size over,max %u\r\n",FINSH_CMD_SIZE);
				gtShell->line_position = 0;
			}

			/* normal character */
			gtShell->line[gtShell->line_position] = ch; 
			ch = 0;
			
			if (gtShell->echo_mode) 
				sh_printf("%c", gtShell->line[gtShell->line_position]);
			gtShell->line_position ++;
			gtShell->use_history = 0; /* it's a new command */
		} /* end of device read */
	}
}

void finsh_system_function_init(const void* begin, const void* end)
{
	_syscall_table_begin = (struct finsh_syscall*) begin;
	_syscall_table_end = (struct finsh_syscall*) end;
}

void finsh_system_var_init(const void* begin, const void* end)
{
	_sysvar_table_begin = (struct finsh_sysvar*) begin;
	_sysvar_table_end = (struct finsh_sysvar*) end;
}

#if defined(__ICCARM__)               /* for IAR compiler */
  #ifdef FINSH_USING_SYMTAB
    #pragma section="FSymTab"
    #pragma section="VSymTab"
  #endif
#endif

//+++++++++++++finsh 命令行入口++++++++++++++++

/*
 * @ingroup finsh
 *
 * This function will initialize finsh shell
 */
void finsh_system_init(void)
{
	//rt_err_t result;
//finsh_heap_init(); 
		INT8U err;
	
#ifdef FINSH_USING_SYMTAB
#ifdef __CC_ARM                 /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    extern const int VSymTab$$Base;
    extern const int VSymTab$$Limit;
	finsh_system_function_init(&FSymTab$$Base, &FSymTab$$Limit);
	finsh_system_var_init(&VSymTab$$Base, &VSymTab$$Limit);
#elif defined (__ICCARM__)      /* for IAR Compiler */
    finsh_system_function_init(__section_begin("FSymTab"),
                               __section_end("FSymTab"));
    finsh_system_var_init(__section_begin("VSymTab"),
                          __section_end("VSymTab"));
#elif defined (__GNUC__)        /* GNU GCC Compiler */
	extern const int __fsymtab_start;
	extern const int __fsymtab_end;
	extern const int __vsymtab_start;
	extern const int __vsymtab_end;
	finsh_system_function_init(&__fsymtab_start, &__fsymtab_end);
	finsh_system_var_init(&__vsymtab_start, &__vsymtab_end);
#endif
#endif





	/* create or set shell structure */
#ifdef RT_USING_HEAP
	gtShell = (struct finsh_shell*)finsh_heap_allocate(sizeof(struct finsh_shell));
#else
	gtShell = &_shell;
#endif
	if (gtShell == RT_NULL)
	{
		sh_printf("no memory for shell\r\n");
		return;
	}
	
	memset(gtShell, 0, sizeof(struct finsh_shell));
   err = OSTaskCreateExt((void (*)(void *)) finsh_thread_entry,           /* Create the start task                                */
                    (void           *) 0,//arg
                    (OS_STK         *)&FINSH_Task_Stk[FINSH_CFG_TASK_STK_SIZE - 1],
                    (INT8U           ) FINSH_TASK_PRIO,//prio
                    (INT16U          ) FINSH_TASK_PRIO,//id
                    (OS_STK         *)&FINSH_Task_Stk[0],
                    (INT32U          ) FINSH_CFG_TASK_STK_SIZE,
                    (void           *) 0,
                    (INT16U          )(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR));
    if(err != OS_ERR_NONE)
      sh_printf("command line task create fail,%d !\r\n",err);
	#if OS_TASK_NAME_EN > 0u
	OSTaskNameSet(FINSH_TASK_PRIO,"cmd line task",&err);
	#endif
	sh_printf("cmd line ready! \r\n");
}
