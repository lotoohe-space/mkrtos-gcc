/*
 * Copyright (c) 2017 Simon Goldschmidt
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
 * Author: Simon Goldschmidt
 *
 */


#include <lwip/opt.h>
#include <lwip/arch.h>
#if !NO_SYS
#include "sys_arch.h"
#include "mkrtos/mem.h"

#endif
#include <lwip/stats.h>
#include <lwip/debug.h>
#include <lwip/sys.h>
#include <mkrtos/task.h>
#include <string.h>

const void * const pvNullPointer = (mem_ptr_t*)0xffffffff;

u32_t sys_jiffies(void)
{
  return sysTasks.sysRunCount;
}

u32_t sys_now(void)
{
  return sysTasks.sysRunCount;
}

void sys_init(void)
{
}

#if !NO_SYS

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    struct sem_hdl * sem_h;
    sem_h= sem_create(count,65535);
    if(!sem_h){
        return ERR_MEM;
    }
    *sem=sem_h;
    return ERR_OK;
}

void sys_sem_free(sys_sem_t *sem)
{
    LWIP_ASSERT("sem != NULL", sem != NULL);
    sem_free(*sem);
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    if(sem==NULL){
        sem=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值
    }
    LWIP_ASSERT("sem != NULL", sem != NULL);
    (*sem)=0;
}

/* semaphores are 1-based because RAM is initialized as 0, which would be valid */
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
    int ret;
    if(sem==NULL)sem=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值
    LWIP_ASSERT("sem != NULL", sem != NULL);
    if(!timeout){
        ret=sem_pend(*sem,0xffffffff);
        if(ret<0){
            return SYS_ARCH_TIMEOUT;

        }
    }else{
        ret=sem_pend(*sem,timeout);
        if(ret<0){
            return SYS_ARCH_TIMEOUT;

        }
    }
    return ret;
}

void
sys_sem_signal(sys_sem_t *sem)
{
    if(sem==NULL)sem=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值
    LWIP_ASSERT("sem != NULL", sem != NULL);
    sem_post(*sem);
}

void
sys_arch_msleep(u32_t delay_ms)
{
    extern int sys_nanosleep(const struct timespec *req, struct timespec *rem);
    struct timespec tims;
    tims.tv_nsec=1000*1000*(delay_ms%1000);
    tims.tv_sec=delay_ms/1000;
    sys_nanosleep(&tims,NULL);
}
#if !LWIP_COMPAT_MUTEX
err_t
sys_mutex_new(sys_mutex_t *mutex)
{
  LWIP_ASSERT("mutex != NULL", mutex != NULL);
  *mutex = mutex_create();
  return ERR_OK;
}

void
sys_mutex_free(sys_mutex_t *mutex)
{
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    mutex_free(*mutex);
}

void
sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    if(mutex==NULL)mutex=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    *mutex=0;
}

void
sys_mutex_lock(sys_mutex_t *mutex)
{
    if(mutex==NULL)mutex=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值
    LWIP_ASSERT("mutex != NULL", mutex != NULL);
    mutex_lock(*mutex,0xffffffff);
}

void
sys_mutex_unlock(sys_mutex_t *mutex)
{
	if(mutex==NULL)mutex=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值 
  LWIP_ASSERT("mutex != NULL", mutex != NULL);
	mutex_unlock(*mutex);
}
#endif

sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
#include <sched.h>
	extern int32_t sys_clone(int (*fn)(void*),void* child_stack,int flags,void* arg);
    int ret;
    ret=sys_clone(function,NULL,CLONE_FS|CLONE_VM|CLONE_FILES|CLONE_PARENT,arg);

	return ret;
}

err_t
sys_mbox_new(sys_mbox_t *mbox, int size)
{
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    LWIP_ASSERT("size >= 0", size >= 0);
    *mbox=msg_create(size,4);
    if(*mbox==0){
        return ERR_MEM;
    }
    return ERR_OK;
}

void
sys_mbox_free(sys_mbox_t *mbox)
{
  /* parameter check */
  LWIP_ASSERT("mbox != NULL", mbox != NULL);
  msg_free(*mbox);
}

void
sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    if(*mbox==NULL)*mbox=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值
    LWIP_ASSERT("mbox != NULL", mbox != NULL);
    *mbox=0;
}

void
sys_mbox_post(sys_mbox_t *q, void *msg)
{
	if(*q==NULL)*q=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值
 
	msg_put(*q,(uint8_t*)&msg,0xFFFFFFFF);
}

err_t
sys_mbox_trypost(sys_mbox_t *q, void *msg)
{
	if(*q==NULL)*q=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值 
	
	if(msg_put(*q,(uint8_t*)&msg,0)<0){
		return ERR_MEM;
	}
  return ERR_OK;
}

err_t
sys_mbox_trypost_fromisr(sys_mbox_t *q, void *msg)
{
  return sys_mbox_trypost(q, msg);
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t *q, void **msg, u32_t timeout)
{
    int ret;
	if(*q==NULL)*q=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值 
	if(timeout==0){
		ret=msg_get(*q,(uint8_t*)msg,0xffffffff);
        if(ret<0){
            return SYS_ARCH_TIMEOUT;
        }
	}else{
		ret=msg_get(*q,(uint8_t*)msg,timeout);
        if(ret<0){
			return SYS_ARCH_TIMEOUT;
		}
	}
	return ret;
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t *q, void **msg)
{
	if(*q==NULL)*q=(void*)&pvNullPointer;//当msg为空时 msg等于pvNullPointer指向的值 
	if(msg_get(*q,(uint8_t*)(msg),0)<0){
		return ERR_MEM;
	}
	return ERR_OK;
}

//检查一个消息邮箱是否有效
//*mbox:消息邮箱
//返回值:1,有效.
//      0,无效
int sys_mbox_valid(sys_mbox_t *mbox)
{  
	if( *mbox==0){
		return 0;
	}
	return 1; 
} 
//查询一个信号量的状态,无效或有效
//sem:信号量指针
//返回值:1,有效.
//      0,无效
int sys_sem_valid(sys_sem_t *sem)
{
	if(*sem==0){
		return 0;
	}
	return 1;           
} 
#if LWIP_NETCONN_SEM_PER_THREAD
#error LWIP_NETCONN_SEM_PER_THREAD==1 not supported
#endif /* LWIP_NETCONN_SEM_PER_THREAD */

#endif /* !NO_SYS */
