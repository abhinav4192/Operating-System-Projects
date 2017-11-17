#include "types.h"
#include "stat.h"
#include "user.h"
#define PGSIZE 4096

// Thread Library

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