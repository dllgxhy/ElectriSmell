
/*
 * File      : finsh_vm.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2010, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2010-03-22     Bernard      first version
 */
#include <finsh.h>

#include "finsh_vm.h"
#include "finsh_ops.h"
#include "finsh_var.h"

/* stack */
union finsh_value	finsh_vm_stack[FINSH_STACK_MAX];
/* text segment */
u_char				text_segment[FINSH_TEXT_MAX];

union finsh_value*	finsh_sp;		/* stack pointer */
u_char*				finsh_pc;		/* PC */

/* syscall list, for dynamic system call register */
struct finsh_syscall_item* global_syscall_list = NULL;

#ifdef RT_USING_HEAP
extern void* finsh_heap_allocate(size_t size);
#endif

// #define VM_DISASSEMBLE
void finsh_vm_run()
{
	u_char op;

	/* if want to disassemble the bytecode, please define VM_DISASSEMBLE */
#ifdef VM_DISASSEMBLE
	void finsh_disassemble();
	finsh_disassemble();
#endif

	/* set sp(stack pointer) to the beginning of stack */
	finsh_sp = &finsh_vm_stack[0];

	/* set pc to the beginning of text segment */
	finsh_pc = &text_segment[0];

	while ((finsh_pc - &text_segment[0] >= 0) &&
		(finsh_pc - &text_segment[0] < FINSH_TEXT_MAX))
	{
		/* get op */
		op = *finsh_pc++;

		/* call op function */
		op_table[op]();
	}
}

#ifdef RT_USING_HEAP
extern char *strdup(const char *s);
void finsh_syscall_append(const char* name, syscall_func func)
{
	/* create the syscall */
	struct finsh_syscall_item* item;

	item = (struct finsh_syscall_item*)finsh_heap_allocate(sizeof(struct finsh_syscall_item));
	if (item != RT_NULL)
	{
		item->next = NULL;
		item->syscall.name = strdup(name);
		item->syscall.func = func;

		if (global_syscall_list == NULL)
		{
			global_syscall_list = item;
		}
		else
		{
			item->next = global_syscall_list;
			global_syscall_list = item;
		}
	}
}
#endif

struct finsh_syscall* finsh_syscall_lookup(const char* name)
{
	struct finsh_syscall* index;
	struct finsh_syscall_item* item;

	for (index = _syscall_table_begin; index < _syscall_table_end; index ++)
	{
		if (strcmp(index->name, name) == 0)
			return index;
	}

	/* find on syscall list */
	item = global_syscall_list;
	while (item != NULL)
	{
		if (strncmp(item->syscall.name, name, strlen(name)) == 0)
		{
			return &(item->syscall);
		}

		item = item->next;
	}

	return NULL;
}

#ifdef VM_DISASSEMBLE
void finsh_disassemble()
{
	u_char *pc, op;

	pc = &text_segment[0];
	while (*pc != 0)
	{
		op = *pc;
		switch (op)
		{
		case FINSH_OP_ADD_BYTE:
			pc ++;
			sh_printf("addb\n");
			break;

		case FINSH_OP_SUB_BYTE:
			pc ++;
			sh_printf("subb\n");
			break;

		case FINSH_OP_DIV_BYTE:
			pc ++;
			sh_printf("divb\n");
			break;

		case FINSH_OP_MOD_BYTE:
			pc ++;
			sh_printf("modb\n");
			break;

		case FINSH_OP_MUL_BYTE:
			pc ++;
			sh_printf("mulb\n");
			break;

		case FINSH_OP_AND_BYTE:
			pc ++;
			sh_printf("andb\n");
			break;

		case FINSH_OP_OR_BYTE:
			pc ++;
			sh_printf("orb\n");
			break;

		case FINSH_OP_XOR_BYTE:
			pc ++;
			sh_printf("xorb\n");
			break;

		case FINSH_OP_BITWISE_BYTE:
			pc ++;
			sh_printf("bwb\n");
			break;

		case FINSH_OP_SHL_BYTE:
			pc ++;
			sh_printf("shlb\n");
			break;

		case FINSH_OP_SHR_BYTE:
			pc ++;
			sh_printf("shrb\n");
			break;

		case FINSH_OP_LD_BYTE:
			pc ++;
			sh_printf("ldb %d\n", *pc++);
			break;

		case FINSH_OP_LD_VALUE_BYTE:
			pc ++;
			sh_printf("ldb [0x%x]\n", FINSH_GET32(pc));
			pc += 4;
			break;

		case FINSH_OP_ST_BYTE:
			pc ++;
			sh_printf("stb\n");
			break;

		case FINSH_OP_ADD_WORD:
			pc ++;
			sh_printf("addw\n");
			break;

		case FINSH_OP_SUB_WORD:
			pc ++;
			sh_printf("subw\n");
			break;

		case FINSH_OP_DIV_WORD:
			pc ++;
			sh_printf("divw\n");
			break;

		case FINSH_OP_MOD_WORD:
			pc ++;
			sh_printf("modw\n");
			break;

		case FINSH_OP_MUL_WORD:
			pc ++;
			sh_printf("mulw\n");
			break;

		case FINSH_OP_AND_WORD:
			pc ++;
			sh_printf("andw\n");
			break;

		case FINSH_OP_OR_WORD:
			pc ++;
			sh_printf("orw\n");
			break;

		case FINSH_OP_XOR_WORD:
			pc ++;
			sh_printf("xorw\n");
			break;

		case FINSH_OP_BITWISE_WORD:
			pc ++;
			sh_printf("bww\n");
			break;

		case FINSH_OP_SHL_WORD:
			pc ++;
			sh_printf("shlw\n");
			break;

		case FINSH_OP_SHR_WORD:
			pc ++;
			sh_printf("shrw\n");
			break;

		case FINSH_OP_LD_WORD:
			pc ++;
			sh_printf("ldw %d\n", FINSH_GET16(pc));
			pc += 2;
			break;

		case FINSH_OP_LD_VALUE_WORD:
			pc ++;
			sh_printf("ldw [0x%x]\n", FINSH_GET32(pc));
			pc += 4;
			break;

		case FINSH_OP_ST_WORD:
			pc ++;
			sh_printf("stw\n");
			break;

		case FINSH_OP_ADD_DWORD:
			pc ++;
			sh_printf("addd\n");
			break;

		case FINSH_OP_SUB_DWORD:
			pc ++;
			sh_printf("subd\n");
			break;

		case FINSH_OP_DIV_DWORD:
			pc ++;
			sh_printf("divd\n");
			break;

		case FINSH_OP_MOD_DWORD:
			pc ++;
			sh_printf("modd\n");
			break;

		case FINSH_OP_MUL_DWORD:
			pc ++;
			sh_printf("muld\n");
			break;

		case FINSH_OP_AND_DWORD:
			pc ++;
			sh_printf("andd\n");
			break;

		case FINSH_OP_OR_DWORD:
			pc ++;
			sh_printf("ord\n");
			break;

		case FINSH_OP_XOR_DWORD:
			pc ++;
			sh_printf("xord\n");
			break;

		case FINSH_OP_BITWISE_DWORD:
			pc ++;
			sh_printf("bwd\n");
			break;

		case FINSH_OP_SHL_DWORD:
			pc ++;
			sh_printf("shld\n");
			break;

		case FINSH_OP_SHR_DWORD:
			pc ++;
			sh_printf("shrd\n");
			break;

		case FINSH_OP_LD_DWORD:
			pc ++;
			sh_printf("ldd 0x%x\n", FINSH_GET32(pc));
			pc += 4;
			break;

		case FINSH_OP_LD_VALUE_DWORD:
			pc ++;
			sh_printf("ldd [0x%x]\n", FINSH_GET32(pc));
			pc += 4;
			break;

		case FINSH_OP_ST_DWORD:
			pc ++;
			sh_printf("std\n");
			break;

		case FINSH_OP_POP:
			sh_printf("pop\n");
			pc ++;
			break;

		case FINSH_OP_SYSCALL:
			pc ++;
			sh_printf("syscall %d\n", *pc++);
			break;

		case FINSH_OP_LD_VALUE_BYTE_STACK:
			pc ++;
			sh_printf("ldb [sp]\n");
			break;

		case FINSH_OP_LD_VALUE_WORD_STACK:
			pc ++;
			sh_printf("ldw [sp]\n");
			break;

		case FINSH_OP_LD_VALUE_DWORD_STACK:
			pc ++;
			sh_printf("ldd [sp]\n");
			break;

		default:
			return;
		}
	}
}
#endif
