#include "types.h"
#include "stat.h"
#include "user.h"
#include "x86.h"

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

int thread_create(void (*start_routine)(void*), void *arg){
    printf(1,"thread_create\n");
    void *t_stack= malloc(PGSIZE);
    printf(1,"thread_create: Stack Location : %x\n",t_stack);
    return -1;
}


int thread_join(){
    printf(1,"thread_join\n");
    return -1;
}