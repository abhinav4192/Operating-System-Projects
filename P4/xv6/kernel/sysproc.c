#include "types.h"
#include "x86.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "sysfunc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return proc->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since boot.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_clone(void){
    // cprintf("sys_clone\n");
    void(*fcn)(void*);
    void *arg;
    void* stack;
    if(argptr(0, (void *)&fcn, sizeof(*fcn)) < 0 || argptr(1, (void *)&arg, sizeof(*arg))
     || argptr(2, (void *)&stack, sizeof(*stack))) {
        cprintf("sys_clone error\n");
        return -1;
    }
    // else{
    //     cprintf("fcn: %d, arg: %d, stack %d \n",fcn,arg,stack);
    // }
    return clone(fcn,arg,stack);
}

int sys_join(void){
    // cprintf("sys_join\n");
    void **stack;
    if(argptr(0, (void *)&stack, sizeof(*stack))) {
        cprintf("sys_join error\n");
        return -1;
    }
    // cprintf("sys_join stack:%d\n",*stack);
    return join(stack);;
}

int sys_cwait(void){
    // cprintf("sys_cwait\n");
    cond_t * iCond;
    lock_t * iLock;
    if(argptr(0, (void *)&iCond, sizeof(*iCond)) || argptr(1, (void *)&iLock, sizeof(*iLock))) {
        cprintf("sys_cwait error\n");
        return -1;
    }
    // else{
    //     cprintf("sys_cwait iCond: %d, iLock: %d\n",iCond,iLock);
    // }
    cwait(iCond,iLock);
    return 0;
}

int sys_csignal(void){
    // cprintf("sys_csignal\n");
    cond_t * iCond;
    if(argptr(0, (void *)&iCond, sizeof(*iCond))) {
        cprintf("sys_csignal error\n");
        return -1;
    }
    // else{
    //     cprintf("sys_csignal iCond: %d\n",iCond);
    // }
    csignal(iCond);
    return 0;
}
