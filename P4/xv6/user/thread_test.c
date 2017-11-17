#include "types.h"
#include "stat.h"
#include "user.h"


void create_01(){
    thread_create(NULL,NULL);
}

void join_01(){
    thread_join();
}


int main(int argc, char *argv[])
{
    create_01();
    join_01();
    exit();
}