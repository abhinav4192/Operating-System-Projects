#include "types.h"
#include "stat.h"
#include "user.h"

int global = 0;
int d = 1;
cond_t t;
lock_t l;

void test_thread_01(void* arg_ptr){
    for(int i=0 ;i < 10;i++){
        global++;
    }
}

void test_thread_02(void* arg_ptr){
    lock_acquire(&l);
    cond_wait(&t,&l);
    lock_release(&l);
}


void cv_test(){
    cond_init(&t);
    lock_init(&l);
    // Lock not acquired
    // cond_wait(&t,&l);

    for(int i=0;i<8;i++){
        printf(1,"cv_test: %d\n",i+1);
        thread_create(test_thread_02,(void*)&d);
    }
}

void zombie_test(){
    for(int i=0;i<15;i++){
        printf(1,"zombie_test: %d\n",i+1);
        thread_create(test_thread_01,(void*)&d);
    }
    // for(int i=0;i<15;i++){
    //     printf(1,"zombie_test: %d\n",i+1);
    //     thread_join();
    // }
}

int main(int argc, char *argv[])
{

    // for (int i=1;i<=50;i++){
    //     printf(1,"Running loop: %d\n",i);
    //     for(int j=0;j<8;j++)
    //         thread_create(test_thread_01,(void*)&d);
    //     for(int j=0;j<8;j++)
    //         thread_join();
    // }

    // cv_test();
    zombie_test();
    exit();
}