/**************************************************************************/ 
/*                                                                        */ 
/*            Copyright (c) 1996-2016 by Express Logic Inc.               */ 
/*                                                                        */ 
/*  This Original Work may be modified, distributed, or otherwise used in */ 
/*  any manner with no obligations other than the following:              */ 
/*                                                                        */ 
/*    1. This legend must be retained in its entirety in any source code  */ 
/*       copies of this Work.                                             */ 
/*                                                                        */ 
/*    2. This software may not be used in the development of an operating */
/*       system product.                                                  */ 
/*                                                                        */  
/*  This Original Work is hereby provided on an "AS IS" BASIS and WITHOUT */ 
/*  WARRANTY, either express or implied, including, without limitation,   */ 
/*  the warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A  */ 
/*  PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY OF this         */ 
/*  ORIGINAL WORK IS WITH the user.                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc. reserves the right to modify this software        */ 
/*  without notice.                                                       */ 
/*                                                                        */ 
/*  Express Logic, Inc.                     info@expresslogic.com         */
/*  11423 West Bernardo Court               http://www.expresslogic.com   */
/*  San Diego, CA  92127                                                  */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** Thread-Metric Component                                               */
/**                                                                       */
/**   Porting Layer (Must be completed with RTOS specifics)               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary files.  */

#include    <stdlib.h>
#include    <kernel.h>
#include    <shell/shell.h>
#include    <app_memory/app_memdomain.h>
#include    <misc/libc-hooks.h>
#include    <arch/arm/cortex_m/cmsis.h>
#include    "tm_api.h"

LOG_MODULE_REGISTER(thread_metric);

static void tm_thread_task(intptr_t exinf);

K_THREAD_DEFINE(TM_TASK_0, 512/*STACKSIZE*/, tm_thread_task, 0, NULL, NULL, 1, K_USER, K_FOREVER);
K_THREAD_DEFINE(TM_TASK_1, 512/*STACKSIZE*/, tm_thread_task, 1, NULL, NULL, 1, K_USER, K_FOREVER);
K_THREAD_DEFINE(TM_TASK_2, 512/*STACKSIZE*/, tm_thread_task, 2, NULL, NULL, 1, K_USER, K_FOREVER);
K_THREAD_DEFINE(TM_TASK_3, 512/*STACKSIZE*/, tm_thread_task, 3, NULL, NULL, 1, K_USER, K_FOREVER);
K_THREAD_DEFINE(TM_TASK_4, 512/*STACKSIZE*/, tm_thread_task, 4, NULL, NULL, 1, K_USER, K_FOREVER);
K_THREAD_DEFINE(TM_TASK_5, 512/*STACKSIZE*/, tm_thread_task, 5, NULL, NULL, 1, K_USER, K_FOREVER);

K_MSGQ_DEFINE(TM_MSGQ, 16, 1, 4);

K_SEM_DEFINE(TM_SEM, 1, 1);

typedef struct {
    k_tid_t   taskid;
    void      *entry_function;
} thread_t;

K_APP_DMEM(part_tm) static thread_t thread_tab[] = {
	{TM_TASK_0}, {TM_TASK_1}, {TM_TASK_2}, {TM_TASK_3}, {TM_TASK_4}, {TM_TASK_5}
};

static void
tm_thread_task(intptr_t exinf)
{
	((void (*)(void)) thread_tab[exinf].entry_function)();
}


/* This function called from main performs basic RTOS initialization, 
   calls the test initialization function, and then starts the RTOS function.  */
void  tm_initialize(void (*test_initialization_function)(void))
{
	(test_initialization_function)();
}


/* This function takes a thread ID and priority and attempts to create the
   file in the underlying RTOS.  Valid priorities range from 1 through 31, 
   where 1 is the highest priority and 31 is the lowest. If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.   */
int  tm_thread_create(int thread_id, int priority, void (*entry_function)(void))
{
	thread_tab[thread_id].entry_function = entry_function;
    k_thread_priority_set(thread_tab[thread_id].taskid, priority);
    k_thread_start(thread_tab[thread_id].taskid);
	tm_thread_suspend(thread_id);
	return TM_SUCCESS;
}


/* This function resumes the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_thread_resume(int thread_id)
{
    k_thread_resume(thread_tab[thread_id].taskid);
	return TM_SUCCESS;
}


/* This function suspends the specified thread.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_thread_suspend(int thread_id)
{
    k_thread_suspend(thread_tab[thread_id].taskid);
	return TM_SUCCESS;
}


/* This function relinquishes to other ready threads at the same
   priority.  */
void tm_thread_relinquish(void)
{
    k_yield();
}


/* This function suspends the specified thread for the specified number
   of seconds.  If successful, the function should return TM_SUCCESS. 
   Otherwise, TM_ERROR should be returned.  */
void tm_thread_sleep(int seconds)
{
    k_sleep(seconds * 1000U);
}


/* This function creates the specified queue.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_create(int queue_id)
{
	return TM_SUCCESS;
}


/* This function sends a 16-byte message to the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_send(int queue_id, unsigned long *message_ptr)
{
    k_msgq_put(&TM_MSGQ, message_ptr, K_FOREVER);
	return TM_SUCCESS;
}


/* This function receives a 16-byte message from the specified queue.  If successful, 
   the function should return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_queue_receive(int queue_id, unsigned long *message_ptr)
{
    k_msgq_get(&TM_MSGQ, message_ptr, K_FOREVER);
	return TM_SUCCESS;
}


/* This function creates the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_create(int semaphore_id)
{
	return TM_SUCCESS;
}


/* This function gets the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_get(int semaphore_id)
{
    if (k_sem_take(&TM_SEM, K_NO_WAIT) == 0) {
		return TM_SUCCESS;
    }
	return TM_ERROR;
}


/* This function waits the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_wait(int semaphore_id)
{
    if (k_sem_take(&TM_SEM, K_FOREVER) == 0) {
		return TM_SUCCESS;
    }
	return TM_ERROR;
}


/* This function puts the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_put(int semaphore_id)
{
    k_sem_give(&TM_SEM);
	return TM_SUCCESS;
}


/* This function puts the specified semaphore.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_semaphore_put_from_isr(int semaphore_id)
{
    k_sem_give(&TM_SEM);
	return TM_SUCCESS;
}


/* This function creates the specified memory pool that can support one or more
   allocations of 128 bytes.  If successful, the function should
   return TM_SUCCESS. Otherwise, TM_ERROR should be returned.  */
int  tm_memory_pool_create(int pool_id)
{
	return TM_SUCCESS;
}


/* This function allocates a 128 byte block from the specified memory pool.  
   If successful, the function should return TM_SUCCESS. Otherwise, TM_ERROR 
   should be returned.  */
int  tm_memory_pool_allocate(int pool_id, unsigned char **memory_ptr)
{
    *memory_ptr = malloc(128);
	return TM_SUCCESS;
}


/* This function releases a previously allocated 128 byte block from the specified 
   memory pool. If successful, the function should return TM_SUCCESS. Otherwise, TM_ERROR 
   should be returned.  */
int  tm_memory_pool_deallocate(int pool_id, unsigned char *memory_ptr)
{
    free(memory_ptr);
	return TM_SUCCESS;
}

void tm_interrupt_raise(void)
{
    NVIC->STIR = I2C3_ER_IRQn;
}

#define ENABLE_PRINTF

#if defined(CONFIG_ISOTEE_GUEST)
#define printf LOG_ERR
#endif

#define main tm_basic_processing_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_basic_processing_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_basic_processing_array[1024];
#include "tm_basic_processing_test.c"
#undef main

#define main tm_cooperative_scheduling_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_cooperative_thread_0_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_cooperative_thread_1_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_cooperative_thread_2_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_cooperative_thread_3_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_cooperative_thread_4_counter;
#include "tm_cooperative_scheduling_test.c"
#undef main

#define main tm_interrupt_preemption_processing_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_interrupt_preemption_thread_0_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_interrupt_preemption_thread_1_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_interrupt_preemption_handler_counter;
#include "tm_interrupt_preemption_processing_test.c"
#undef main

#define main tm_interrupt_processing_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_interrupt_thread_0_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_interrupt_handler_counter;
#include "tm_interrupt_processing_test.c"
#undef main

#define main tm_memory_allocation_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_memory_allocation_counter;
#include "tm_memory_allocation_test.c"
#undef main

#define main tm_message_processing_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_message_processing_counter;
extern K_APP_BMEM(part_tm) unsigned long   tm_message_sent[4];
extern K_APP_BMEM(part_tm) unsigned long   tm_message_received[4];
#include "tm_message_processing_test.c"
#undef main

#define main tm_preemptive_scheduling_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_preemptive_thread_0_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_preemptive_thread_1_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_preemptive_thread_2_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_preemptive_thread_3_counter;
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_preemptive_thread_4_counter;
#include "tm_preemptive_scheduling_test.c"
#undef main

#define main tm_synchronization_processing_test_main
extern K_APP_BMEM(part_tm) volatile unsigned long   tm_synchronization_processing_counter;
#include "tm_synchronization_processing_test.c"
#undef main

K_APP_BMEM(part_tm) static void *test_interrupt_handler;

void
tm_interrupt_handler()
{
	((void (*)(void)) test_interrupt_handler)();
}

K_APPMEM_PARTITION_DEFINE(part_tm);

K_APP_DMEM(part_tm) static char tm_main_task_input = '-';

struct k_mem_domain dom_tm;

void
tm_main_task(intptr_t exinf)
{
	printf("Thread-Metric Test Suite starts.\n");

	switch (tm_main_task_input) {
	case '1':
		printf("START: Basic Processing Test\n");
		tm_basic_processing_test_main();
		break;

	case '2':
		printf("START: Cooperative Scheduling Test\n");
		tm_cooperative_scheduling_test_main();
		break;

	case '3':
		printf("START: Preemptive Scheduling Test\n");
		tm_preemptive_scheduling_test_main();
		break;

	case '4':
		printf("START: Interrupt Processing Test\n");
		test_interrupt_handler = tm_interrupt_processing_handler;
		tm_interrupt_processing_test_main();
		break;

	case '5':
		printf("START: Interrupt Preemption Processing Test\n");
		test_interrupt_handler = tm_interrupt_preemption_handler;
		tm_interrupt_preemption_processing_test_main();
		break;

	case '6':
		printf("START: Message Processing Test\n");
		tm_message_processing_test_main();
		break;

	case '7':
		printf("START: Synchronization Processing Test\n");
		tm_synchronization_processing_test_main();
		break;

	case '8':
		printf("START: Memory Allocation Test");
		tm_memory_allocation_test_main();
		break;

	default:
		printf("Unknown command: '%c'.", tm_main_task_input);
	}
    k_thread_abort(k_current_get());
}

K_THREAD_DEFINE(TM_MAIN, 512/*STACKSIZE*/, tm_main_task, NULL, NULL, NULL, CONFIG_MAIN_THREAD_PRIORITY, K_USER, K_FOREVER);

int cmd_tm(const struct shell *shell, size_t argc, char **argv) {
	ARG_UNUSED(argc);

    tm_main_task_input = argv[1][0];

#if !defined(CONFIG_ISOTEE_GUEST)
    SCB->CCR |= SCB_CCR_USERSETMPEND_Msk;
#endif

    IRQ_CONNECT(I2C3_ER_IRQn, _EXC_IRQ_DEFAULT_PRIO, tm_interrupt_handler, NULL, 0);
    irq_enable(I2C3_ER_IRQn);

#if defined(CONFIG_USERSPACE)
    k_mem_domain_init(&dom_tm, 0, NULL);
    k_mem_domain_add_partition(&dom_tm, &part_tm);
    k_mem_domain_add_partition(&dom_tm, &z_malloc_partition);
    k_mem_domain_add_thread(&dom_tm, TM_MAIN);
    k_mem_domain_add_thread(&dom_tm, TM_TASK_0);
    k_mem_domain_add_thread(&dom_tm, TM_TASK_1);
    k_mem_domain_add_thread(&dom_tm, TM_TASK_2);
    k_mem_domain_add_thread(&dom_tm, TM_TASK_3);
    k_mem_domain_add_thread(&dom_tm, TM_TASK_4);
    k_mem_domain_add_thread(&dom_tm, TM_TASK_5);
#endif

    k_thread_access_grant(TM_MAIN, TM_TASK_0, TM_TASK_1, TM_TASK_2, TM_TASK_3, TM_TASK_4, TM_TASK_5, &TM_MSGQ, &TM_SEM);
    k_thread_access_grant(TM_TASK_0, TM_TASK_0, TM_TASK_1, TM_TASK_2, TM_TASK_3, TM_TASK_4, TM_TASK_5, &TM_MSGQ, &TM_SEM);
    k_thread_access_grant(TM_TASK_1, TM_TASK_0, TM_TASK_1, TM_TASK_2, TM_TASK_3, TM_TASK_4, TM_TASK_5, &TM_MSGQ, &TM_SEM);
    k_thread_access_grant(TM_TASK_2, TM_TASK_0, TM_TASK_1, TM_TASK_2, TM_TASK_3, TM_TASK_4, TM_TASK_5, &TM_MSGQ, &TM_SEM);
    k_thread_access_grant(TM_TASK_3, TM_TASK_0, TM_TASK_1, TM_TASK_2, TM_TASK_3, TM_TASK_4, TM_TASK_5, &TM_MSGQ, &TM_SEM);
    k_thread_access_grant(TM_TASK_4, TM_TASK_0, TM_TASK_1, TM_TASK_2, TM_TASK_3, TM_TASK_4, TM_TASK_5, &TM_MSGQ, &TM_SEM);
    k_thread_access_grant(TM_TASK_5, TM_TASK_0, TM_TASK_1, TM_TASK_2, TM_TASK_3, TM_TASK_4, TM_TASK_5, &TM_MSGQ, &TM_SEM);

    k_thread_start(TM_MAIN);
    k_sleep(70000U);
    k_thread_abort(TM_MAIN);
    k_thread_abort(TM_TASK_0);
    k_thread_abort(TM_TASK_1);
    k_thread_abort(TM_TASK_2);
    k_thread_abort(TM_TASK_3);
    k_thread_abort(TM_TASK_4);
    k_thread_abort(TM_TASK_5);
    printf("Thread-Metric Test Suite exits.\n");

	return 0;
}

SHELL_CMD_ARG_REGISTER(tm, NULL,
"1. Basic Processing Test\n"
"2. Cooperative Scheduling Test\n"
"3. Preemptive Scheduling Test\n"
"4. Interrupt Processing Test\n"
"5. Interrupt Preemption Processing Test\n"
"6. Message Processing Test\n"
"7. Synchronization Processing Test\n"
"8. Memory Allocation Test\n"
, cmd_tm, 2, 0);

