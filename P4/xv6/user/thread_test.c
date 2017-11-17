#include "types.h"
#include "stat.h"
#include "user.h"

int global = 0;

void test_thread_01(){
    for(int i=0 ;i < 100000;i++){
        global++;
    }
}

void create_01(){
    thread_create(test_thread_01,NULL);
}

void join_01(){
    thread_join();
}


int main(int argc, char *argv[])
{
    for(int i=0;i<10;i++){
        create_01();
    }
    for(int i=0;i<4;i++){
        thread_join();
    }
    printf(1,"global: %d\n",global);
    exit();
}