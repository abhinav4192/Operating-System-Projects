#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>

#define SHM_NAME "abhinav_bidyut"
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define MAXCLIENTS (PAGE_SIZE/(64) -1)
#define SEG_SZ  64
/****************************************************************CLIENT          CLIENT          CLIENT    ***********/
// Mutex variables
pthread_mutex_t* mutex;
pthread_mutexattr_t mutexAttribute;
int MyIndex=0;
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
	pthread_mutex_lock(mutex);
	memset(shm_mem + MyIndex*SEG_SZ, 0, sizeof(int));
	pthread_mutex_unlock(mutex);
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc != 2 || !argv[1])
		exit(0);
	char *clientstring = argv[1];
	int clientstr_sz = strlen(clientstring);
	if(clientstr_sz > 9)
		exit(0);
	// Creating a new shared memory segment
	int fd_shm = shm_open(SHM_NAME, O_RDWR, 0660);
	if(fd_shm == -1){
		exit(0);
	}
	shm_mem = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
	if(!shm_mem)
		exit(0);
		
	mutex = (pthread_mutex_t *)(shm_mem + SEG_SZ*MAXCLIENTS);
	stats_t statistics;
	
	pthread_mutex_lock(mutex);
	int full=1;	
	for(int i=0;i<=MAXCLIENTS;i++){
		if(*(int *)(shm_mem+i*SEG_SZ) == 0){
			MyIndex = i;
			statistics.status = 1;
			full =0;
			memcpy(shm_mem + MyIndex*SEG_SZ, &statistics.status, sizeof(int));
			break;
		}	
	
	}
	if(full){
		pthread_mutex_unlock(mutex);
		exit(-1);
	}	
		
	pthread_mutex_unlock(mutex);
	struct timeval start_time;
	struct timeval cur_time;
	int res;
	res  = gettimeofday(&start_time, NULL);
	time_t t;
	t = time(NULL);
		
	if(res==0){
	
	}
	statistics.pid = getpid();
	strncpy(statistics.birth, ctime(&t), sizeof(statistics.birth));
	statistics.birth[24] = '\0';
	
	strncpy(statistics.clientString, argv[1], clientstr_sz); 	

	struct sigaction sigact;
	sigact.sa_handler = exit_handler;
	sigaction(SIGTERM, &sigact, 0);
	sigaction(SIGINT, &sigact, 0);
	
    
	while (1)
	{
		gettimeofday(&cur_time, NULL);
		statistics.elapsed_sec = cur_time.tv_sec - start_time.tv_sec;
		statistics.elapsed_msec = (cur_time.tv_usec - start_time.tv_usec)/1000;
		memcpy(shm_mem + MyIndex*SEG_SZ, &statistics, sizeof(stats_t));
		sleep(1);
		printf("Active clients :");
		for(int j=0;j<=MAXCLIENTS;j++){
			if(*(int *)(shm_mem+j*SEG_SZ) == 1){
				printf(" %d",*((int *)(shm_mem+j*SEG_SZ) + 1));
			}
		}
		printf("\n");
        }

    return 0;
}
