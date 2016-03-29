/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *         Simon Goldschmidt
 *
 */

#include <stdlib.h>
#ifdef SUPPORT_NETWORK

#include <stdio.h> /* sprintf() for task names */
#include <string.h>


#include "include/opt.h"
#include "include/arch.h"
#include "include/stats.h"
#include "include/debug.h"
#include "include/sys.h"

#include "akos_api.h"
#include "fwl_oscom.h"
#include "fwl_osmalloc.h"
#include "eng_debug.h"


/* These functions are used from NO_SYS also, for precise timer triggering */
T_U32 sys_start_time = 0;

void sys_init_timing()
{
  sys_start_time = Fwl_GetTickCount();
}

static T_U32 sys_get_ms_longlong()
{
  T_U32 ret;
  T_U32 now;

  now = Fwl_GetTickCount();
  ret = now - sys_start_time;
  return ret;
}

u32_t sys_jiffies()
{
  return (u32_t)sys_get_ms_longlong();
}

u32_t sys_now()
{
  return (u32_t)sys_get_ms_longlong();
}

T_hSemaphore critSec = AK_INVALID_SEMAPHORE;


void InitSysArchProtect()
{
  critSec = AK_Create_Semaphore(1, AK_PRIORITY);
}
u32_t sys_arch_protect()
{
  AK_Obtain_Semaphore(critSec, AK_SUSPEND);
  return 0;
}
void sys_arch_unprotect(u32_t pval)
{
  AK_Release_Semaphore(critSec);
}

void FreeSysArchProtect()
{
  AK_Delete_Semaphore(critSec);
}


void msvc_sys_init()
{
  sys_init_timing();
  InitSysArchProtect();
}

void sys_init()
{
  msvc_sys_init();
}

void sys_free()
{
  FreeSysArchProtect();
}


#if !NO_SYS

struct threadlist {
  T_hTask task;
  T_pVOID pStackAddr;
  struct threadlist *next;
};

struct threadlist *lwip_threads = NULL;

void do_sleep(int ms)
{
  AK_Sleep(ms/5);
}


err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	T_hSemaphore new_sem = AK_INVALID_SEMAPHORE;

	LWIP_ASSERT("sem != NULL", sem != NULL);

	new_sem = AK_Create_Semaphore(count, AK_PRIORITY);
	LWIP_ASSERT("Error creating semaphore", new_sem != AK_INVALID_SEMAPHORE);
	if (new_sem != AK_INVALID_SEMAPHORE)
	{
		SYS_STATS_INC_USED(sem);
#if LWIP_STATS && SYS_STATS
    	LWIP_ASSERT("sys_sem_new() counter overflow", lwip_stats.sys.sem.used != 0 );
#endif /* LWIP_STATS && SYS_STATS*/
    	sem->sem = new_sem;
		return ERR_OK;
	}
   
  /* failed to allocate memory... */
  SYS_STATS_INC(sem.err);
  sem->sem = SYS_SEM_NULL;
  return ERR_MEM;
}

void sys_sem_free(sys_sem_t *sem)
{
  /* parameter check */
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != SYS_SEM_NULL", sem->sem != SYS_SEM_NULL);
  AK_Delete_Semaphore(sem->sem);

  SYS_STATS_DEC(sem.used);
#if LWIP_STATS && SYS_STATS
  LWIP_ASSERT("sys_sem_free() closed more than created", lwip_stats.sys.sem.used != (u16_t)-1);
#endif /* LWIP_STATS && SYS_STATS */
  sem->sem = SYS_SEM_NULL;
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
  T_S32 ret;
  T_U32 starttime, endtime;
  
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != SYS_SEM_NULL", sem->sem != SYS_SEM_NULL);

  if(!timeout)
  {
    /* wait infinite */
    starttime = sys_get_ms_longlong();
	ret = AK_Obtain_Semaphore(sem->sem, AK_SUSPEND);
    LWIP_ASSERT("Error waiting for semaphore", ret == AK_SUCCESS);
    endtime = sys_get_ms_longlong();
    /* return the time we waited for the sem */
    return (u32_t)(endtime - starttime);
  }
  else
  {
    starttime = sys_get_ms_longlong();
    ret = AK_Obtain_Semaphore(sem->sem, timeout);
    LWIP_ASSERT("Error waiting for semaphore", (ret == AK_SUCCESS) || (ret == AK_TIMEOUT));
    if(ret == AK_SUCCESS)
    {
      endtime = sys_get_ms_longlong();
      /* return the time we waited for the sem */
      return (u32_t)(endtime - starttime);
    }
    else
    {
      /* timeout */
      return SYS_ARCH_TIMEOUT;
    }
  }
}

void sys_sem_signal(sys_sem_t *sem)
{
  T_S32 ret;
  LWIP_ASSERT("sem != NULL", sem != NULL);
  LWIP_ASSERT("sem->sem != SYS_SEM_NULL", sem->sem != SYS_SEM_NULL);
  ret = AK_Release_Semaphore(sem->sem);
  LWIP_ASSERT("Error releasing semaphore", ret == AK_SUCCESS);
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
  struct threadlist *new_thread;
  SYS_ARCH_DECL_PROTECT(lev);

  LWIP_UNUSED_ARG(name);
  LWIP_UNUSED_ARG(stacksize);
  LWIP_UNUSED_ARG(prio);

  new_thread = (struct threadlist*)Fwl_Malloc(sizeof(struct threadlist));
  LWIP_ASSERT("new_thread != NULL", new_thread != NULL);

  new_thread->pStackAddr = Fwl_Malloc(stacksize);
  LWIP_ASSERT("pStackAddr != NULL", new_thread->pStackAddr != NULL);	
	memset(new_thread->pStackAddr, 0, stacksize);
	
  if(new_thread != NULL 
  	&& new_thread->pStackAddr != NULL) 
  {
    SYS_ARCH_PROTECT(lev);
    new_thread->next = lwip_threads;
    lwip_threads = new_thread;

	new_thread->task = AK_Create_Task((T_VOID*)function, name, 1, arg, new_thread->pStackAddr, stacksize, prio, 5, AK_PREEMPT, AK_START);
    LWIP_ASSERT("task > 0", new_thread->task > 0);

    SYS_ARCH_UNPROTECT(lev);
    return new_thread->task;
  }
  return 0;
}


err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{	
  T_hSemaphore new_sem = AK_INVALID_SEMAPHORE;

  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_UNUSED_ARG(size);

  new_sem = AK_Create_Semaphore(0, AK_PRIORITY);
  LWIP_ASSERT("Error creating semaphore", new_sem != AK_INVALID_SEMAPHORE);
  if(new_sem == AK_INVALID_SEMAPHORE) {
    SYS_STATS_INC(mbox.err);
	mbox->sem = SYS_SEM_NULL;
    return ERR_MEM;
  }
  
  mbox->sem = new_sem;
  memset(&mbox->q_mem, 0, sizeof(u32_t)*MAX_QUEUE_ENTRIES);
  mbox->head = 0;
  mbox->tail = 0;
  SYS_STATS_INC_USED(mbox);
#if LWIP_STATS && SYS_STATS
  LWIP_ASSERT("sys_mbox_new() counter overflow", lwip_stats.sys.mbox.used != 0 );
#endif /* LWIP_STATS && SYS_STATS */
  return ERR_OK;
}

void sys_mbox_free(sys_mbox_t *mbox)
{
  /* parameter check */
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  LWIP_ASSERT("mbox->sem != NULL", mbox->sem != NULL);

  AK_Delete_Semaphore(mbox->sem);

   SYS_STATS_DEC(mbox.used);
#if LWIP_STATS && SYS_STATS
   LWIP_ASSERT( "sys_mbox_free() ", lwip_stats.sys.mbox.used!= (u16_t)-1 );
#endif /* LWIP_STATS && SYS_STATS */
  mbox->sem = NULL;
}

void sys_mbox_post(sys_mbox_t *q, void *msg)
{
  T_S32 ret;
  SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  SYS_ARCH_PROTECT(lev);
  
  q->q_mem[q->head] = msg;
  (q->head)++;
  if (q->head >= MAX_QUEUE_ENTRIES) {
    q->head = 0;
  }
  LWIP_ASSERT("mbox is full!", q->head != q->tail);
  ret = AK_Release_Semaphore(q->sem);
  LWIP_ASSERT("Error releasing sem", ret == AK_SUCCESS);

  SYS_ARCH_UNPROTECT(lev);
}

err_t sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
  u32_t new_head;
  T_S32 ret;

  SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  SYS_ARCH_PROTECT(lev);

  new_head = q->head + 1;
  if (new_head >= MAX_QUEUE_ENTRIES) {
    new_head = 0;
  }
  if (new_head == q->tail) {
    SYS_ARCH_UNPROTECT(lev);
    return ERR_MEM;
  }

  q->q_mem[q->head] = msg;
  q->head = new_head;
  LWIP_ASSERT("mbox is full!", q->head != q->tail);
  ret = AK_Release_Semaphore(q->sem);
  LWIP_ASSERT("Error releasing sem", ret == AK_SUCCESS);

  SYS_ARCH_UNPROTECT(lev);
  
  return ERR_OK;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
  T_S32 ret;
  T_U32 suspend = AK_SUSPEND;
  T_U32 starttime, endtime;
  
  SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);

  if (0 == timeout)
  {
	suspend = AK_SUSPEND;
  }
  else if (0 == timeout % 5)
  {
	suspend = timeout / 5;
  }
  else
  {
	suspend = timeout / 5 + 1;
  }
  
  starttime = sys_get_ms_longlong();

  if ((ret = AK_Obtain_Semaphore(q->sem, suspend)) == AK_SUCCESS) {
    SYS_ARCH_PROTECT(lev);
    if(msg != NULL
		&& q->q_mem[q->tail] != 0) 
	{
      *msg  = q->q_mem[q->tail];
	  
	  (q->tail)++;
	    if (q->tail >= MAX_QUEUE_ENTRIES) {
	      q->tail = 0;
	    }
    }

    SYS_ARCH_UNPROTECT(lev);
    endtime = sys_get_ms_longlong();

	if (endtime < starttime)
	{
		AK_DEBUG_OUTPUT("*********endtime < starttime******%lu, %lu*****\n", endtime, starttime);
		return (0xffffffff - starttime + 1 + endtime);
	}
	
	return (u32_t)(endtime - starttime);
  }
  else
  {
    //LWIP_ASSERT("Error waiting for sem", ret == AK_TIMEOUT);
    if(msg != NULL) {
      *msg  = NULL;
    }

    return SYS_ARCH_TIMEOUT;
  }
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
  T_S32 ret;
  
  SYS_ARCH_DECL_PROTECT(lev);

  /* parameter check */
  LWIP_ASSERT("q != SYS_MBOX_NULL", q != SYS_MBOX_NULL);
  LWIP_ASSERT("q->sem != NULL", q->sem != NULL);
  

  if ((ret = AK_Obtain_Semaphore(q->sem, 0)) == AK_SUCCESS) {
    SYS_ARCH_PROTECT(lev);
    if(msg != NULL
		&& q->q_mem[q->tail] != 0) 
	{
      *msg  = q->q_mem[q->tail];

	  (q->tail)++;
	    if (q->tail >= MAX_QUEUE_ENTRIES) {
	      q->tail = 0;
	    }
    }

    
    SYS_ARCH_UNPROTECT(lev);

    return 0;
  }
  else
  {
    //LWIP_ASSERT("Error waiting for sem", ret == AK_TIMEOUT);
    if(msg != NULL) {
      *msg  = NULL;
    }

    return SYS_ARCH_TIMEOUT;
  }
}

#endif /* !NO_SYS */

#endif
