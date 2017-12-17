#include <pthread.h>
#include <stdio.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <unistd.h>
#include "name_server.h"
#include "job_queue.h"
#include <stdbool.h>
#define ARGNUM 1
#define PORT "50000"

pthread_mutex_t master_mutex = PTHREAD_MUTEX_INITIALIZER;

fd_set socket_set_glob;

int listener;
bool listening = false;
int max_fd;

void* worker(void* arg);

int main(int argc, char**argv) {
  if (argc != ARGNUM + 1) {
    printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
    return(0);
  }
  printf("Starting server..\n");

  FD_ZERO(&socket_set_glob);

  int num_threads = atoi(argv[1]);

  struct addrinfo hints, *addri_res, *tmp_addr;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int addr_err, sock_flag;
  if ((addr_err = getaddrinfo(NULL, PORT, &hints, &addri_res)) != 0) {
    fprintf(stderr, "selectserver: %s\n", gai_strerror(addr_err));
    exit(1);
  }

  for (tmp_addr = addri_res; tmp_addr != NULL; tmp_addr = tmp_addr->ai_next) {
    listener = socket(tmp_addr->ai_family, tmp_addr->ai_socktype, tmp_addr->ai_protocol);

    if (listener < 0) {
      continue;
    }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &sock_flag, sizeof(int));

    if (bind(listener, tmp_addr->ai_addr, tmp_addr->ai_addrlen) < 0)  {
      close(listener);
      continue;
    }
    break;
  }

  if (tmp_addr == NULL) {
    fprintf(stderr, "Failed to bind to socket\n");
    exit(5);
  }

  freeaddrinfo(addri_res);
  if(listen(listener, 15) < 0 ) {
    fprintf(stderr, "Failed to listen on socket\n");
    exit(3);
  }

  FD_SET(listener, &socket_set_glob);

  fd_set read_fds;

  struct job_queue jq;
  job_queue_init(&jq, 15);
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));

  max_fd = listener;

  for (int i=0; i<num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &jq) != 0 ) {
      err(1, "pthread_create() failed\n");
    }
  }
  // Start listening
  while (1) {
    read_fds = socket_set_glob; // copy global filedescripter set
    // select available sockets from fdset
    if (select(max_fd+1, &read_fds,NULL, NULL, NULL) == -1) {
      perror("select error\n");
      exit(3);
    }
    else { // iterate over selected sockets
      for (int i=0; i<=max_fd; i++) {
        if (FD_ISSET(i, &read_fds)) {
          int tmp_sock = i;
          // push socket-filedescripter to the jobqueue
          if (i == listener) {
            if (!listening) {
              job_queue_push(&jq, (void*)&tmp_sock);
              listening = true;
            } else {
              continue;
            }
          } else {
            job_queue_push(&jq, (void*)&tmp_sock);
          }
        }
      }
    }
  }


  // Destroy jobqueue
  job_queue_destroy(&jq);
  printf("DESTROYED THE JOBQUEUE!\n");

  // reap/join terminated threads.
  for (int i = 0; i<num_threads; i++)  {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() error\n");
    }
  }
  free(threads);
  return 0;
}

void* worker(void * arg) {
  struct job_queue *jq = arg;
  struct sockaddr client_addr;

  char data_buf[512];  // client data buffer
  int data_size;       // size of client data in bytes
  int* sock; // socket fd to hold job_queue pop.
  while(1) {
    if (job_queue_pop(jq, (void**)&sock) == 0) { // pop a job
      printf("\n\n\nWORKER POPPED SOME WORK!! \n");
      printf("Jobqueuesize: %d\n", jq->num_used);
      if (* sock == listener) { // If the listener, there is a new connection ready.
        printf("Listener^\n");
        unsigned int addr_len = sizeof(client_addr);
        int new_sock = accept(listener, &client_addr, &addr_len);
        listening = false;
        if (new_sock == -1) {
          perror("accept error\n");
        } else { // no error
          pthread_mutex_lock(&master_mutex); // lock the mutex
          FD_SET(new_sock, &socket_set_glob); // Add the the new socket to the global fd set
          if (new_sock > max_fd) {
            max_fd = new_sock;
          }
          printf("Listener accepted new connection with socket %d. \n", new_sock);
          pthread_mutex_unlock(&master_mutex); // unlock the mutex
        }
      } else { // If existing connection with client
        data_size = recv(*sock, data_buf, sizeof(data_buf), 0); // recieve bytes from client and store in buffer
        printf("Found an existing connection w/ socket %d\n", *sock);
        if (data_size <= 0) { // If we didn't recieve any data or error
          if (data_size==0){
            printf("server: socket %d hung up\n", *sock);
          } else {
            perror("recv() error\n");
          }
          close(*sock); // close socket
          pthread_mutex_lock(&master_mutex); // lock the mutex
          FD_CLR(*sock, &socket_set_glob); // remove socket from set
          pthread_mutex_unlock(&master_mutex); // unlock mutex
        } else { // Data is received
          printf ("Recieved data: \n %s", (char*)data_buf);
        }
      }
    } else {
      break;
    }
  } // end while
  printf("End of work\n");
  return NULL;
}
