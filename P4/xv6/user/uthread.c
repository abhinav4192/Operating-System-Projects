#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

#define PGSIZE 4096
#define NTHREADS 8

int lock_init(lock_t *ilock){
	ilock->locked = 0;
	return 0;
}

void lock_acquire(lock_t *ilock){
	while(xchg(&ilock->locked, 1) != 0);
}

void lock_release(lock_t *ilock){
	xchg(&ilock->locked, 0);
}

int is_lock_acquired(lock_t *ilock){
    return ilock->locked;
}

int thread_create(void (*start_routine)(void*), void *arg){
    // printf(1,"thread_create\n");
    void *t_stack= malloc(PGSIZE);
    if((uint)t_stack == 0) return -1;
    int t_id = clone(start_routine,arg,t_stack);
    // printf(1, "thread_create: t_id: %d\n",t_id);
    return t_id;
}

int thread_join(){
    // printf(1,"thread_join\n");
    void *t_stack = (void*) 0;
    int t_id = join(&t_stack);
    if(t_id != -1) free(t_stack);
    return t_id;
}

void
panic_cv(char *s)
{
    printf(1, "panic: %s\n", s);
    exit();
}

void cond_init(cond_t *iCond){
    // printf(1,"cond_init\n");
    if(iCond==0) panic_cv("Condition variable is not valid");
    lock_init(&iCond->qlock);
    lock_acquire(&iCond->qlock);
    iCond->token=0;
    iCond->curr=0;
    lock_release(&iCond->qlock);
}

void cond_wait(cond_t *iCond, lock_t *ilock){
    // printf(1,"cond_wait\n");
    if(iCond==0) panic_cv("Condition variable is not valid");
    if(ilock == 0) panic_cv("Lock is not valid");
    if(is_lock_acquired(ilock)==0) panic_cv("Lock is not acquired");
    lock_acquire(&iCond->qlock);
    if(iCond->token - iCond->curr >= NTHREADS){
        lock_release(&iCond->qlock);
        panic_cv("Condition variable wait queue is full");
    }
    iCond->token++;
    cwait(iCond,ilock);
    // printf(1,"cond_wait:iCond->token: %d\n",iCond->token);
}

void cond_signal(cond_t *iCond){
    // printf(1,"cond_signal\n");
    if(iCond==0) panic_cv("Condition variable is not valid");
    lock_acquire(&iCond->qlock);
    if(iCond->curr < iCond->token){
        // printf(1,"cond_signal:iCond->curr: %d\n",iCond->curr);
        iCond->curr++;
        csignal(iCond);
    }else{
        lock_release(&iCond->qlock);
    }

}