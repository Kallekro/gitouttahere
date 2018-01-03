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
pthread_mutex_t conn_info_mutex = PTHREAD_MUTEX_INITIALIZER;

//struct socket_struct* conn_array;

fd_set master_set;
fd_set tmp_set;
struct connection_info* conn_info_array;
int conn_count;

int listener;
bool listening = false;
int max_fd;

const char* cmdlist[4] = { "/login", "/logout", "/exit", "/lookup" };
const int cmdlen = 4;
int parsecmd(char* input);

void* worker(void* arg);
int handle_login(struct connection_info* conn_info, char* input);

int main(int argc, char**argv) {
  if (argc != ARGNUM + 1) {
    printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
    return(0);
  }

  //set_connection_info(&(connection_info[0]), "user1", "pass1", ""
  printf("Starting server..\n");

  int num_threads = atoi(argv[1]);

  struct addrinfo hints, *addri_res, *tmp_addr;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int addr_err, sock_flag = 1;
  if ((addr_err = getaddrinfo(NULL, PORT, &hints, &addri_res)) != 0) {
    fprintf(stderr, "server: %s\n", gai_strerror(addr_err));
    exit(1);
  }

  for (tmp_addr = addri_res; tmp_addr != NULL; tmp_addr = tmp_addr->ai_next) {
    listener = socket(tmp_addr->ai_family, tmp_addr->ai_socktype, tmp_addr->ai_protocol);

    if (listener < 0) {
      continue;
    }

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &sock_flag, sizeof(int));
    //setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &sock_flag, sizeof(int));

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

  // for non-blocking
  //int flags = fcntl(listener, F_GETFL, 0);
  //fcntl(listener, F_SETFL, flags | O_NONBLOCK);

  FD_ZERO(&master_set);
  FD_ZERO(&tmp_set);
  
  FD_SET(listener, &master_set);
  max_fd = listener;

  //conn_array = malloc(conn_array_len * sizeof(struct socket_struct));
  int conn_array_len = 100; 
  conn_info_array = malloc(conn_array_len * sizeof(struct connection_info));
  conn_count = 0; 

  for (int i=0; i<100; i++) {
    set_connection_info(&conn_info_array[i], NULL , NULL, NULL, NULL);
  }

  //for (int i = 0; i < conn_array_len; i++) {
  //  reset_socket(&(conn_array[i]));
  //}
  //set_socket(&conn_array[0], listener, 0);

  struct job_queue jq;
  job_queue_init(&jq, 15);
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));

  for (int i=0; i<num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &jq) != 0 ) {
      err(1, "pthread_create() failed\n");
    }
  }

  struct sockaddr client_addr;
  int new_sock;
  // main loop
  while (1) {
    pthread_mutex_lock(&master_mutex);
    tmp_set = master_set;
    pthread_mutex_unlock(&master_mutex);

    if (select(max_fd+1, &tmp_set, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(1);
    }

    for (int i=0; i <= max_fd; i++) {
      if (FD_ISSET(i, &tmp_set)) {
        if (i == listener) { // new connection
          unsigned int addr_len = sizeof(client_addr);
          new_sock = accept(listener, &client_addr, &addr_len);
          printf("accepted new connection\n");
          if (new_sock == -1) {
            perror("accept");
          }
        } else { // existing connection
          pthread_mutex_lock(&master_mutex);
          new_sock = i;
          FD_CLR(i, &master_set);  
          pthread_mutex_unlock(&master_mutex);
        }
        // add the readable socket to the job queue
        job_queue_push(&jq, &new_sock);
      }
    }


   // for (int i=0; i<conn_count; i++) {
   //   if (!conn_array[i].busy && conn_array[i].sockfd != -1) {
   //     conn_array[i].busy = 1;
   //     job_queue_push(&jq, &(conn_array[i]));
   //   }
   // }
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
  close(listener);

  //for (int i = 0; i < conn_array_len; i++) {
  //  if (conn_array[i].sockfd != -1) {
  //    close(conn_array[i].sockfd);
  //  }
  //}
  //free(conn_array);

  return 0;
}

void* worker(void * arg) {
  struct job_queue *jq = arg;

  char data_buf[512];  // client data buffer
  int data_size;       // size of client data in bytes

  int* sock;
  while(1) {
    if (job_queue_pop(jq, (void**)&sock) == 0) { // pop a job

      pthread_mutex_lock(&conn_info_mutex);

      if (!conn_info_array[*sock].nick) {
        // A new connection

        data_size = recv(*sock, data_buf, sizeof(data_buf), 0);
        data_buf[data_size] = '\0';
        // TODO: CHECK IF CORRECT LOGIN
        char loginmsg[100];
        if (handle_login(&(conn_info_array[*sock]), data_buf) == 0) {
          sprintf(loginmsg, "Login succesful. Welcome %s\n", conn_info_array[*sock].nick);
          send(*sock, "success", 10, 0);
        } else {
          sprintf(loginmsg, "Login unsuccesful. Please check you login information and try again\n");
          send(*sock, "fail", 10, 0);
          printf("close sock\n");
          close(*sock);
          continue;
        }

      pthread_mutex_unlock(&conn_info_mutex);

      pthread_mutex_lock(&master_mutex);
      FD_SET(*sock, &master_set);
      pthread_mutex_unlock(&master_mutex);
      }
      //if (sock->sockfd == listener) { // If the listener, there is a new connection ready.
      //  unsigned int addr_len = sizeof(client_addr);
      //  int new_sock = accept(listener, &client_addr, &addr_len);
      //  if (new_sock == -1) {
      //    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      //      perror("accept error\n");
      //    }
      //  } else { 
      //    //int flags = fcntl(new_sock, F_GETFL, 0);
      //    //fcntl(new_sock, F_SETFL, flags | O_NONBLOCK);

      //    pthread_mutex_lock(&master_mutex); // lock the mutex

      //    data_size = recv(new_sock, data_buf, sizeof(data_buf), 0);
      //    // TODO: CHECK IF CORRECT LOGIN
      //    char loginmsg[100];
      //    if (handle_login(&(conn_info_array[conn_count]), data_buf) == 0) {
      //      sprintf(loginmsg, "Login succesful. Welcome %s\n", conn_info_array[conn_count].nick);
      //      send(new_sock, "success", 10, 0);
      //    } else {
      //      sprintf(loginmsg, "Login unsuccesful. Please check you login information and try again\n");
      //      send(new_sock, "fail", 10, 0);
      //      printf("close sock\n");
      //      close(new_sock);
      //      pthread_mutex_unlock(&master_mutex );
      //      sock->busy = 0;
      //      continue;
      //    }

      //    set_socket(&(conn_array[conn_count]), new_sock, 0); 
      //    conn_count++;

      //    printf("Listener accepted new connection with socket %d. \n", new_sock);
      //    pthread_mutex_unlock(&master_mutex); // unlock the mutex
      //  }
      //} else { // If existing connection with client
      //  data_size = recv(sock->sockfd, data_buf, sizeof(data_buf), MSG_PEEK); // recieve bytes from client and store in buffer
      //  if (data_size != -1) {
      //    printf("data size: %d", data_size);
      //  }
      //  if (data_size == 0) { // If we didn't recieve any data or error
      //    printf("server: socket %d hung up\n", sock->sockfd);
      //    close(sock->sockfd); // close socket
      //    pthread_mutex_lock(&master_mutex); // lock the mutex 
      //    for (int i = 0; i < conn_count; i++) {
      //      if (sock->sockfd == conn_array[i].sockfd) {
      //        if (i == conn_count-1) {
      //          reset_socket(&(conn_array[i]));
      //        } else {
      //          conn_array[i] = conn_array[conn_count-1];
      //        }
      //        break;
      //      }
      //    }
      //    conn_count--;

      //    pthread_mutex_unlock(&master_mutex); // unlock mutex
      //  } else { // Data is received:w

      //    if (data_size > 0) {
      //      data_buf[data_size] = '\0';
      //      printf ("Recieved data: \n %s", (char*)data_buf);
      //      switch (parsecmd(data_buf)) {
      //        case 0:
      //          printf("LOGIN\n");
      //          break;
      //        case 1:
      //          printf("LOGOUT\n");
      //          break;
      //        case 2:
      //          printf("EXIT\n");
      //          break;
      //        case 3:
      //          printf("LOOKUP\n");
      //          break;
      //      }
      //      send(sock->sockfd, "LOL", 3, 0);
      //    }
      //  }
      //}
      //sock->busy = 0;
    } else {
      printf("wat\n");
      break;
    }
  } // end while
  printf("End of work\n");
  return NULL;
}

int parsecmd(char* input) {
  char cmpbuf[24];
  for (int i = 0; i < cmdlen; i++) {
    int len = strlen(cmdlist[i]);
    strncpy(cmpbuf, input, len);
    cmpbuf[len] = '\0';
    if (strcmp(cmpbuf, cmdlist[i]) == 0) {
      return i;
    }
  } 
  return -1;
}

int handle_login (struct connection_info* ci, char* input) {
  int spaces[4];
  int i = 0, spacecount = 0;
  printf("input: %s\n", input);
  while (input[i] != '\0') {
    if (input[i] == ' ') {
      spaces[spacecount] = i;
      spacecount++;
    }
    i++;
  }

  if (spacecount != 4) {
    return -1;
  }
  char _nick[256];
  char _passwd[256];
  char _ip[256];
  char _port[256];
  strncpy(_nick, input + spaces[0]+1, spaces[1] - spaces[0] - 1);
  strncpy(_passwd, input + spaces[1]+1, spaces[2] - spaces[1] - 1);
  strncpy(_ip, input + spaces[2]+1, spaces[3] - spaces[2] - 1);
  strncpy(_port, input + spaces[3]+1, (int)strlen(input) - spaces[3] - 2);
  set_connection_info(ci, _nick, _passwd, _ip, _port);
  return 0;
}

