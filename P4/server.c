#include "cs537.h"
#include "request.h"

//
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too

int front = -1;
int back = -1;
pthread_cond_t empty, full;
pthread_mutex_t mutex;
int clientbufferlen, threads_count;

//Wrapper functions for pthreads functions used.

int Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr){
  int rc;

  if ((rc = pthread_mutex_init(mutex, attr)) != 0)
      unix_error("Mutex Initialization error");
  return rc;
}

int Pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr){
  int rc;

  if ((rc = pthread_cond_init(cond, attr)) != 0)
      unix_error("Condition variable Initialization error");
  return rc;
}

int Pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){
  int rc;

  if ((rc = pthread_create(thread, attr, start_routine, arg)) != 0)
      unix_error("Pthread Creation error");
  return rc;
}

int Pthread_mutex_lock(pthread_mutex_t *mutex)
{
    int rc;

    if ((rc = pthread_mutex_lock(mutex)) != 0)
        unix_error("Mutex lock error");
    return rc;
}

int Pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    int rc;

    if ((rc = pthread_mutex_unlock(mutex)) != 0)
        unix_error("Mutex unlock error");
    return rc;
}

int Pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    int rc;

    if ((rc = pthread_cond_wait(cond,mutex)) != 0)
        unix_error("Mutex condition wait error");
    return rc;
}

int Pthread_cond_signal(pthread_cond_t *cond)
{
    int rc;

    if ((rc = pthread_cond_signal(cond)) != 0)
        unix_error("Mutex condition signal error");
    return rc;
}

int Pthread_join(pthread_t thread, void **value_ptr)
{
    int rc;

    if ((rc = pthread_join(thread, value_ptr)) != 0)
        unix_error("Pthread join error");
    return rc;
}
/*   Wrapper functions end */

void getargs(int *port, int *bufferlen, int *threads_count, int argc, char *argv[])
{
    if (argc != 4) {
	     fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
	     exit(1);
    }

    *port = atoi(argv[1]);
    *bufferlen = atoi(argv[3]);
    *threads_count = atoi(argv[2]);
    if(*bufferlen <=0 || *threads_count <= 0 || *port <0){
      fprintf(stderr, "Please provide a positive value for port, threads and buffers\n");
      exit(1);
    }


}
int isFull(){
  if((back == clientbufferlen -1 && front == 0) || (back == front -1)){
    //Buffer is full
    return 1;
  }
  return 0;
}
int isEmpty(){
  if(front == -1){
    return 1;
  }
  return 0;
}


int insertIntoBuffer(int connecfd, int client_bufferfd[])
{
  //Already check if buffer is full so assume that buffer has space.
  if(front == -1){
    front = back = 0;
    //Ist connection into buffer
    client_bufferfd[back] = connecfd;
  }
  else if(front && back == clientbufferlen -1){
    //wrap around
    back =0;
    client_bufferfd[back] = connecfd;
  }
  else{
    //Nth connection into buffer
    //back++;
    client_bufferfd[++back] = connecfd;
  }
  return 0;
}

int removeFromQueue(int client_bufferfd[]){

  //Already checked whether empty or not, so assume queue has connections.
  int fd = client_bufferfd[front];
  client_bufferfd[front] = 0;
  if(front==back){
    //Empty queue again
    front = back = -1;
  }
  else if(front == clientbufferlen-1){
    //wrap around front
    front = 0;
  } else{
    //Creating space at front
    front++;
  }

  return fd;
}
void *workerThread(void *arg){

  //Handle the request in the worked thread.
  int *client_bufferfd = (int *)arg;
  while(1){
    Pthread_mutex_lock(&mutex);
    while(isEmpty()){
      //Queue is currently empty. So going into blocked state
      Pthread_cond_wait(&full, &mutex);
    }

    //Signal came. Waking up
    int connfd = removeFromQueue(client_bufferfd);
    Pthread_cond_signal(&empty);
    Pthread_mutex_unlock(&mutex);
    requestHandle(connfd);
    Close(connfd);

  }
  //Shouldnt ideally reach here
  return 0;
}

int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    //global buffer for client requests
    //
    Pthread_mutex_init(&mutex, NULL);
    Pthread_cond_init(&full, NULL);
    Pthread_cond_init(&empty, NULL);

    getargs(&port, &clientbufferlen, &threads_count, argc, argv);
    int client_bufferfd[clientbufferlen];

    //
    // CS537: Create some threads...
    pthread_t Thread[threads_count];

    for(int i=0;i<threads_count; i++){
        Pthread_create(&Thread[i], NULL, workerThread, client_bufferfd);
      }


    listenfd = Open_listenfd(port);
    while (1) {

    	clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
      Pthread_mutex_lock(&mutex);
      while(isFull()){
        Pthread_cond_wait(&empty, &mutex);
      }
      insertIntoBuffer(connfd, client_bufferfd);
      Pthread_cond_signal(&full);
      Pthread_mutex_unlock(&mutex);

    	//
    	// CS537: In general, don't handle the request in the main thread.
    	// Save the relevant info in a buffer and have one of the worker threads
    	// do the work.
    	//

    }

    //Should not ideally reach here.
    for(int i=0;i<threads_count; i++){
      Pthread_join(Thread[i], NULL);
    }

}
