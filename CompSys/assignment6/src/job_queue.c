#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->capacity = capacity;
  job_queue->first = 0;
  job_queue->num_used = 0;
  job_queue->buffer = malloc(sizeof(void*) * capacity);
  
  //pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  //pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


  //pthread_mutex_init(&(mutex), NULL);
  //pthread_cond_init( &(cond), NULL);

  //job_queue->mutex = mutex;
  //job_queue->cond = cond;

  pthread_mutex_init(&(job_queue->mutex), NULL);
  pthread_cond_init(&(job_queue->cond), NULL);

  job_queue->dead = 0;


  printf("Initialized\n");
  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  //pthread_mutex_lock(&(job_queue->mutex));
  pthread_mutex_trylock(&(job_queue->mutex));

  while (job_queue->num_used > 0) {
    pthread_cond_wait(&(job_queue->cond), &(job_queue->mutex));
  }
  
  pthread_mutex_unlock(&(job_queue->mutex));
  free(job_queue->buffer);
  
  pthread_mutex_destroy(&(job_queue->mutex));
  pthread_cond_destroy(&(job_queue->cond));

  job_queue->dead = 1;

  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  //pthread_mutex_lock(&(job_queue->mutex));
  pthread_mutex_trylock(&(job_queue->mutex));
  
  while (job_queue->num_used >= job_queue->capacity) {
    pthread_cond_wait(&(job_queue->cond), &(job_queue->mutex));
  }

  printf("Num: %d\n", job_queue->num_used);
  job_queue->num_used++;
  printf("Num: %d\n", job_queue->num_used);

  int index = job_queue->first + job_queue->num_used;
  if (index > job_queue->capacity) {
    index = 0;
  }
  job_queue->buffer[index] = data;

  pthread_mutex_unlock(&(job_queue->mutex));
  pthread_cond_broadcast(&(job_queue->cond));

  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  //printf("Worker started\n");
  //pthread_mutex_lock(&(job_queue->mutex));
  //printf("error: %s\n", strerror(pthread_mutex_trylock(&(job_queue->mutex))));
  printf("error: %s\n", strerror(pthread_mutex_lock(&(job_queue->mutex))));
  
  printf("Mutex is locked!\n");

  //printf("Worker Num: %d\n", job_queue->num_used);
  while (job_queue->num_used == 0) {
    printf("Worker waits\n");
    pthread_cond_wait(&(job_queue->cond), &(job_queue->mutex));
  }
  //printf("Worker Num: %d\n", job_queue->num_used);
  job_queue->num_used--;
  
  *data = job_queue->buffer[job_queue->first];
  job_queue->first++;

  pthread_mutex_unlock(&(job_queue->mutex));
  printf("Mutex is unlocked!\n");
  pthread_cond_broadcast(&(job_queue->cond));
  
  return 0;
}



