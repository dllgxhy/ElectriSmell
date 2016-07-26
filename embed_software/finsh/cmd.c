/*
 * File      : cmd.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-04-30     Bernard      first implementation
 * 2006-05-04     Bernard      add list_thread,
 *                                 list_sem,
 *                                 list_timer
 * 2006-05-20     Bernard      add list_mutex,
 *                                 list_mailbox,
 *                                 list_msgqueue,
 *                                 list_event,
 *                                 list_fevent,
 *                                 list_mempool
 * 2006-06-03     Bernard      display stack information in list_thread
 * 2006-08-10     Bernard      change version to invoke rt_show_version
 * 2008-09-10     Bernard      update the list function for finsh syscall
 *                                 list and sysvar list
 * 2009-05-30     Bernard      add list_device
 * 2010-04-21     yi.qiu          add list_module
 */


#include "finsh.h"


long help(void)
{
    sh_printf("This projector is create by liaoxinchang !\r\n");
    sh_printf("All rights reserved!\r\n");
    sh_printf("The command line format: fun(args...)  !\r\n");
    sh_printf("same as write a function!\r\n");
	return 0;
}
FINSH_FUNCTION_EXPORT(help, how use the cmd line);
int list(void)
{

	rt_uint16_t func_cnt;
	func_cnt = 0;
	{
		struct finsh_syscall* index;
		for (index = _syscall_table_begin; index < _syscall_table_end; index ++)
		{

			    func_cnt ++;

#ifdef FINSH_USING_DESCRIPTION
				sh_printf("%-16s -- %s\r\n", index->name, index->desc);
#else
				sh_printf("%s\r\n", index->name);
#endif	
		}
	}  
    sh_printf("total command function : %u\r\n", func_cnt);
    
    return 0;
}

FINSH_FUNCTION_EXPORT(list, list all the command function);

static int str_is_prefix(const char* prefix, const char* str)
{
	while ((*prefix) && (*prefix == *str))
	{
		prefix ++;
		str ++;
	}

	if (*prefix == 0) return 0;
	return -1;
}

void list_prefix(char* prefix)
{
	struct finsh_syscall_item* syscall_item;
	struct finsh_sysvar_item*  sysvar_item;
	rt_uint16_t func_cnt, var_cnt;
	const char* name_ptr;

	func_cnt = 0;
	var_cnt  = 0;
	name_ptr = RT_NULL;

	{
		struct finsh_syscall* index;
		for (index = _syscall_table_begin; index < _syscall_table_end; index ++)
		{
			if (str_is_prefix(prefix, index->name) == 0)
			{
				if (func_cnt == 0)
					sh_printf("--function:\r\n");

				func_cnt ++;
				/* set name_ptr */
				name_ptr = index->name;

#ifdef FINSH_USING_DESCRIPTION
				sh_printf("%-16s -- %s\r\n", index->name, index->desc);
#else
				sh_printf("%s\r\n", index->name);
#endif
			}
		}
	}

	/* list syscall list */
	syscall_item = global_syscall_list;
	while (syscall_item != NULL)
	{
		if (str_is_prefix(prefix, syscall_item->syscall.name) == 0)
		{
			if (func_cnt == 0)
				sh_printf("--function:\r\n");
			func_cnt ++;
			/* set name_ptr */
			name_ptr = syscall_item->syscall.name;

			sh_printf("[l] %s\r\n", syscall_item->syscall.name);
		}
		syscall_item = syscall_item->next;
	}

	{
		struct finsh_sysvar* index;
		for (index = _sysvar_table_begin; index < _sysvar_table_end; index ++)
		{
			if (str_is_prefix(prefix, index->name) == 0)
			{
				if (var_cnt == 0)
					sh_printf("--variable:\r\n");

				var_cnt ++;
				/* set name ptr */
				name_ptr = index->name;

#ifdef FINSH_USING_DESCRIPTION
				sh_printf("%-16s -- %s\r\n", index->name, index->desc);
#else
				sh_printf("%s\r\n", index->name);
#endif
			}
		}
	}

	sysvar_item = global_sysvar_list;
	while (sysvar_item != NULL)
	{
		if (str_is_prefix(prefix, sysvar_item->sysvar.name) == 0)
		{
			if (var_cnt == 0)
				sh_printf("--variable:\n");

			var_cnt ++;
			/* set name ptr */
			name_ptr = sysvar_item->sysvar.name;

			sh_printf("[l] %s\n", sysvar_item->sysvar.name);
		}
		sysvar_item = sysvar_item->next;
	}

	/* only one matched */
	if ((func_cnt + var_cnt) == 1)
	{
		strncpy(prefix, name_ptr, strlen(name_ptr));
	}
}

#ifdef FINSH_USING_SYMTAB
static int dummy = 0;
FINSH_VAR_EXPORT(dummy, finsh_type_int, dummy variable for finsh)
#endif

