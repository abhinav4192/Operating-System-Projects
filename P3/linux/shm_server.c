#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>

#define SHM_NAME "abhinav_bidyut"
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE) 
#define MAXCLIENTS (PAGE_SIZE/(64) -1)
#define SEG_SZ  64
/****************************************************************SERVER     SERVER     SERVER    ***********/
// Mutex variables
pthread_mutex_t* mutex;
pthread_mutexattr_t mutexAttribute;
void *shm_mem;

typedef struct {
    int status;
    int pid;
    char birth[25];
    char clientString[10];
    int elapsed_sec;
    double elapsed_msec;
} stats_t;

void exit_handler(int sig)
{
	munmap(shm_mem, PAGE_SIZE);
	shm_unlink(SHM_NAME);
	exit(0);
}

int main(int argc, char *argv[])
{

	// Creating a new shared memory segment
	int fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
 	ftruncate(fd_shm, PAGE_SIZE);
	shm_mem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	stats_t *temp_stat;
    // Initializing mutex
	mutex = (pthread_mutex_t *)(shm_mem + SEG_SZ*MAXCLIENTS); // assigning space for mutex at the first location in shared memory.
	pthread_mutexattr_init(&mutexAttribute);
	pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(mutex, &mutexAttribute);
	struct sigaction sigact;
        sigact.sa_handler = exit_handler;
        sigaction(SIGTERM, &sigact, 0);
        sigaction(SIGINT, &sigact, 0);
	int iteration = 1;
	while (1)
	{
 		sleep(1);
		for(int i =0; i<= MAXCLIENTS;i++){
			
			if(*(int *)(shm_mem+i*SEG_SZ) == 1){
				temp_stat = (stats_t *)(shm_mem+i*SEG_SZ);
				printf("%d, pid : %d, birth : %s, elapsed : %d s %.4f ms, %s\n",iteration,temp_stat->pid, temp_stat->birth,temp_stat->elapsed_sec, temp_stat->elapsed_msec,temp_stat->clientString);
			}
		}
         	
		iteration++;
	}

    return 0;
}
