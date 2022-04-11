/****************************************************************************
 * ARMv7M ELF loader
 * Copyright (c) 2013-2015 Martin Ribelotta
 * Copyright (c) 2019 Johannes Taelman
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>

#include "loader.h"
#include "loader_config.h"
#include "app/sysent.h"
#include "mkrtos/fs.h"
#include "mkrtos/mem.h"
#include "sched.h"
#include <mkrtos/task.h>
#include <arch/arch.h>

#define APP_STACK_SIZE 1048

extern int open(const char *path, int mode, ...);

static const sysent_t sysentries = { /* */
    open, /* */
    close, /* */
    write, /* */
    read, /* */
    printf, /* */
    scanf /* */
};

static const ELFSymbol_t exports[] = { { "syscalls", (void*) &sysentries } };
static const ELFEnv_t env = { exports, sizeof(exports) / sizeof(*exports) };
//执行一个新的程序以替换当前的程序
int sys_execve(const char *filename, char *const argv[ ], char *const envp[ ]){
    uint32_t t;
    int32_t ret;
    //先加载elf
    ELFExec_t *e;
    loader_env_t loader_env;
    loader_env.env = &env;
//    printk("remain memory size is %d.\r\n",GetFreeMemory(1));
    ret=load_elf(filename, loader_env, &e);
    if(ret<0) {
        return -1;
    }
    if (!(e->entry)) {
        MSG("No entry defined.");
        unload_elf(e);
        printk("Done");
        return -1;
    }
    (*(e->used_count))=1;
    int argc_len=0;
    if(argv) {
        while (argv[argc_len]) {
            argc_len++;
        }
    }
    entry_t *entry = (entry_t*) (e->text.data + e->entry);
    //替换当前的进程为新的应用程序
    //替换步骤
    //1.关闭所有的file
    //2.释放所有的内存
    //3.移除所有定时休眠的任务
    //前三个不用关闭
    for( int i=3;i<NR_FILE;i++){
        if(CUR_TASK->files[i].used){
            sys_close(i);
        }
    }


    mem_clear();
    do_remove_sleep_tim(CUR_TASK);
    t=DisCpuInter();

    //execv只是替换程序
    //替换时，如果引用计数没有到0，需要释放掉data，因为data都是独立的
    if(CUR_TASK->exec) {
       (*(CUR_TASK->exec->used_count))--;
        if(!(*(CUR_TASK->exec->used_count))) {
            //卸载之前的elf
            unload_elf(e);
        }else {
//            if(!CUR_TASK->exec->clone_vm) {
                if(CUR_TASK->clone_flag&CLONE_VM) {
                    //没有与父进程使用相同的内存空间，则需要释放这个空间
                    OSFree(CUR_TASK->exec->data.data);
                    OSFree(CUR_TASK->exec->bss.data);
                }
                //fork后的exec也是新的
                OSFree(CUR_TASK->exec);
//            }
        }
    }
    CUR_TASK->exec=e;
    //4.重新初始化栈
    if(CUR_TASK->userStackSize!=0){
        CUR_TASK->skInfo.pspStack=(void*)(&(((uint32_t*)CUR_TASK->memLowStack)[ CUR_TASK->userStackSize-1]));
    }else{
        CUR_TASK->skInfo.pspStack=(void*)(~(0L));
    }
    CUR_TASK->skInfo.mspStack=(void*)(&(((uint32_t*)CUR_TASK->knl_low_stack)[CUR_TASK->kernelStackSize-1]));

    if(CUR_TASK->userStackSize!=0){
        /*设置栈的初始化寄存器*/
        CUR_TASK->skInfo.pspStack=
                OSTaskSetReg(CUR_TASK->skInfo.pspStack,entry,argc_len,argv,envp);
    }
//    CUR_TASK->skInfo.mspStack=
//            OSTaskSetReg(CUR_TASK->skInfo.mspStack,entry,argc_len,argv,envp);

    if(CUR_TASK->userStackSize==0){
        //线程在内核模式;
        CUR_TASK->skInfo.stackType = 2;
    }else{
        CUR_TASK->skInfo.stackType=1;
    }
    CUR_TASK->skInfo.svcStatus=0;
    CUR_TASK->status=TASK_RUNNING;

    CUR_TASK->sig_bmp[0]=0;
    CUR_TASK->sig_bmp[1]=0;
    for(int i=0;i<_NSIG;i++) {
        CUR_TASK->signals[i]._u._sa_handler=SIG_DFL;
    }
    //mem信息不进行fork
    CUR_TASK->mems=NULL;

    sysTasks.isFirst=FALSE;
    //直接进行调度，之后就不会在回来了
    task_sche();
    //设置为初次调度，这样不会保存当前的上下文信息
    SetPSP(0x0);
    RestoreCpuInter(t);

    return ret;
}

void *do_alloc(size_t size, size_t align, ELFSecPerm_t perm) {
  (void) perm;
  /* TODO Change perms with MPU */
  return OSMalloc( size);
}

void *do_alloc_sdram(size_t size, size_t align, ELFSecPerm_t perm) {
  (void) perm;
  // printf("alloc sdram: %8x\n", size);
  /* TODO Change perms with MPU */
  return OSMalloc( size);
}

void arch_jumpTo(entry_t entry,char *const argv[ ], char *const envp[ ]) {
int argc_len=0;
while(argv[argc_len++]);
#if 1
  entry(argc_len,argv,envp);
#else
  void *stack = do_alloc(APP_STACK_SIZE, 8, ELF_SEC_READ | ELF_SEC_WRITE);
  if (stack) {
    register uint32_t saved;
    void *tos = stack + APP_STACK_SIZE;

    /* s->saved */
    __asm__ volatile("MOV %0, sp\n\t" : : "r" (saved));
    /* tos->MSP */
    __asm__ volatile("MOV sp, %0\n\t" : : "r" (tos));
    /* push saved */
    __asm__ volatile("PUSH {%0}\n\t" : : "r" (saved));

    entry();

    /* pop saved */
    __asm__ volatile("POP {%0}\n\t" : : "r" (saved));
    /* saved->sp */
    __asm__ volatile("MOV sp, %0\n\t" : : "r" (saved));

    free(stack);
  } else
    perror("Stack alloc");
#endif
}

int is_streq(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    if (*s1 != *s2)
      return 0;
    s1++;
    s2++;
  }
  return *s1 == *s2;
}
