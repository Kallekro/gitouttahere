#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "job_queue.h"

int job_queue_init(struct job_queue *job_queue, int capacity) {
  job_queue->capacity = capacity;
  job_queue->first = 0;
  job_queue->num_used = 0;
  job_queue->buffer = malloc(sizeof(void*) * capacity);
  job_queue->dead = 0;

  // the mutex is initialized as a normal fast mutex
  pthread_mutex_init(&(job_queue->mutex), NULL);
  // condition is also initialized with default values
  pthread_cond_init(&(job_queue->cond), NULL);

  return 0;
}

int job_queue_destroy(struct job_queue *job_queue) {
  pthread_mutex_lock(&(job_queue->mutex));

  // wait until the job queue is empty
  while (job_queue->num_used > 0) {
    // note that the mutex is unlocked while waiting so other threads can use the queue
    pthread_cond_wait(&(job_queue->cond), &(job_queue->mutex));
  }

  // set dead to true. this will cause every thread waiting to pop() to return -1 after the broadcast
  job_queue->dead = 1;
  free(job_queue->buffer);
  pthread_cond_broadcast(&(job_queue->cond));
  pthread_mutex_unlock(&(job_queue->mutex));

  pthread_mutex_destroy(&(job_queue->mutex));
  pthread_cond_destroy(&(job_queue->cond));

  return 0;
}

int job_queue_push(struct job_queue *job_queue, void *data) {
  pthread_mutex_lock(&(job_queue->mutex));

  // wait until there is room in the job queue if it is filled
  while (job_queue->num_used >= job_queue->capacity) {
    if (job_queue->dead) {
      pthread_mutex_unlock(&(job_queue->mutex));
      return -1;
    }
    pthread_cond_wait(&(job_queue->cond), &(job_queue->mutex));
  }

  int index = job_queue->first + job_queue->num_used;
  // if the index exceeds capacity, wrap around the buffer
  if (index >= job_queue->capacity) {
    index = job_queue->first + job_queue->num_used - job_queue->capacity;
  }

  // after incrementing num_used we should broadcast to let waiting threads check their condition
  job_queue->num_used++;

  // push the data
  job_queue->buffer[index] = data;

  // unlock mutex and broadcast condition
  pthread_cond_broadcast(&(job_queue->cond));
  pthread_mutex_unlock(&(job_queue->mutex));

  return 0;
}

int job_queue_pop(struct job_queue *job_queue, void **data) {
  pthread_mutex_lock(&(job_queue->mutex));

  // wait until there is a job in the queue
  while (job_queue->num_used == 0) {
    // return -1 if the job queue has been killed
    if (job_queue->dead) {
      pthread_mutex_unlock(&(job_queue->mutex));
      return -1;
    }
    pthread_cond_wait(&(job_queue->cond), &(job_queue->mutex));
  }
  // after decrementing we should broadcast to let waiting threads check their condition
  job_queue->num_used--;

  // copy the data to the output
  *data = job_queue->buffer[job_queue->first];
  // increment first element to next one in the queue (possibly wrap around buffer)
  job_queue->first++;
  if (job_queue->first >= job_queue->capacity) {
    job_queue->first = 0;
  }

  // unlock mutex and broadcast condition
  pthread_cond_broadcast(&(job_queue->cond));
  pthread_mutex_unlock(&(job_queue->mutex));

  return 0;
}
