#include <pthread.h>
#include <stdio.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include "name_server.h"
#include "job_queue.h"
#include "socklib.h"
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
int max_conns = 100;

int listener;
bool listening = false;
int max_fd;

const char* cmdlist[4] = { "/login", "/logout", "/exit", "/lookup" };
const int cmdlen = 4;
int parsecmd(char* input);

void* worker(void* arg);
int handle_login(struct connection_info* conn_info, char* input);
int handle_lookup(int sock, char* input);

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
  int conn_array_len = max_conns; 
  conn_info_array = malloc(conn_array_len * sizeof(struct connection_info));
  conn_count = 0; 

  for (int i=0; i<max_conns; i++) {
    set_connection_info(&conn_info_array[i], NULL, NULL, NULL, NULL);
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
    struct timeval timeout;
    timeout.tv_usec = 100;
    timeout.tv_sec = 0;
    if (select(max_fd+1, &tmp_set, NULL, NULL, &timeout) == -1) {
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

  char data_buf[512];  
  int data_size;      
  
  char msgsize[4]; // size of message to send

  int* sock;
  while(1) {
    if (job_queue_pop(jq, (void**)&sock) == 0) { // pop a job
      memset(data_buf, '\0', sizeof(data_buf));
      pthread_mutex_lock(&conn_info_mutex);
      if (!conn_info_array[*sock].nick) {
        // A new connection
        int flags = fcntl(*sock, F_GETFL, 0);
        fcntl(*sock, F_SETFL, flags | O_NONBLOCK);
        data_size = recv_all(*sock, data_buf, sizeof(data_buf));
        data_buf[data_size] = '\0';
        // TODO: CHECK IF CORRECT LOGIN
        char loginmsg[100];
        printf("%d\n", *sock);
        if (handle_login(&(conn_info_array[*sock]), data_buf+4) == 0) {
          printf("lol: %s\n", conn_info_array[*sock].nick);
          strncpy(loginmsg, "1Login succesful. Welcome \0", 30);
          strncpy(loginmsg + strlen(loginmsg), conn_info_array[*sock].nick, 50);
          sprintf(msgsize, "%04lu", strlen(loginmsg));
          send_all(*sock, msgsize, 4);
          send_all(*sock, loginmsg, (int)strlen(loginmsg));
          conn_count += 1;
        } else {
          sprintf(loginmsg, "0Login unsuccesful. Please check you login information and try again\n");
          sprintf(msgsize, "%04lu", strlen(loginmsg));
          send_all(*sock, msgsize, 4);
          send_all(*sock, loginmsg, strlen(loginmsg));
          close(*sock);
          printf("closed connection\n");
          pthread_mutex_unlock(&conn_info_mutex);
          continue;
        }

      pthread_mutex_unlock(&conn_info_mutex);
      

      pthread_mutex_lock(&master_mutex);
      FD_SET(*sock, &master_set);
      if (*sock > max_fd) {
        max_fd = *sock;
      }
      pthread_mutex_unlock(&master_mutex);

      } else {
        pthread_mutex_unlock(&conn_info_mutex);
        if (recv_all(*sock, data_buf, sizeof(data_buf)) == -1) {
          conn_info_array[*sock].nick = NULL;
          close(*sock);
          conn_count--;
          printf("closed connection\n");
          continue;
        }
        char msg[100];
        int remove_sock = 0;
        switch (parsecmd(data_buf+4)) {
          case 0: // Login
            sprintf(msg, "You are already logged in.");
            sprintf(msgsize, "%04lu", strlen(msg));
            send_all(*sock, msgsize, 4);
            send_all(*sock, msg, sizeof(msg));
            break;
          case 1: // Logout
            conn_info_array[*sock].nick = NULL;
            close(*sock);
            conn_count--;
            printf("closed connection\n");
            remove_sock = 1;
            break;
          case 2: // Exit
            conn_info_array[*sock].nick = NULL;
            close(*sock);
            conn_count--;
            printf("closed connection\n");
            remove_sock = 1;
            break;
          case 3: // Lookup
            pthread_mutex_lock(&conn_info_mutex);
            handle_lookup(*sock, data_buf+4);
            pthread_mutex_unlock(&conn_info_mutex);
            break;
          default:
            sprintf(msg, "Unknown command.");
            sprintf(msgsize, "%04lu", strlen(msg));
            send_all(*sock, msgsize, 4);
            send_all(*sock, msg, sizeof(msg));
            break;
        }
        if (!remove_sock) {
          pthread_mutex_lock(&master_mutex);
          FD_SET(*sock, &master_set);
          pthread_mutex_unlock(&master_mutex);
        }
      }
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

int find_spaces (char* input, int* spaces, int maxspaces) {
  int i = 0, spacecount = 0;
  while (input[i] != '\0') {
    if (input[i] == ' ') {
      spaces[spacecount] = i;
      spacecount++;
      if (spacecount > maxspaces-1) {
        return spacecount;
      }
    }
    i++;
  }
  return spacecount;
}

int handle_login (struct connection_info* ci, char* input) {
  int spaces[4];

  if (find_spaces(input, spaces, 4) < 4) {
    return -1;
  }
  char _nick[256];
  char _passwd[256];
  char _ip[256];
  char _port[256];
  strncpy(_nick, input + spaces[0]+1, spaces[1] - spaces[0] - 1);
  _nick[spaces[1] - spaces[0]-1] = '\0';

  strncpy(_passwd, input + spaces[1]+1, spaces[2] - spaces[1] - 1);
  _passwd[spaces[2] - spaces[1]-1] = '\0';
  
  strncpy(_ip, input + spaces[2]+1, spaces[3] - spaces[2] - 1);
  _ip[spaces[3] - spaces[2]-1] = '\0';
  
  strncpy(_port, input + spaces[3]+1, (int)strlen(input) - spaces[3] - 2);
  _port[(int)strlen(input) - spaces[3] - 2] = '\0';

  if (strlen(_nick) < 1 ||
      strlen(_passwd) < 1 ||
      strlen(_ip) < 1 ||
      strlen(_port) < 1) {
    return -1;
  }

  ci->nick = strdup(_nick);
  ci->passwd = strdup(_passwd);
  ci->ip = strdup(_ip);
  ci->port = strdup(_port);
  
  //set_connection_info(ci, _nick, _passwd, _ip, _port);
  return 0;
}

int insert_string(char* dst, char* src, int offset) {
  strcpy(dst+offset, src);
  return strlen(src);
}

int handle_lookup(int sock, char* input) {
  int spaces[1];
  char msgsize[4];
  if (find_spaces(input, spaces, 1) < 1) {
    char conn_count_str[4];
    sprintf(conn_count_str, "%d", conn_count);
    printf("conn count: %d\n", conn_count);
    send_all(sock, "0004", 4);
    send_all(sock, conn_count_str, 4);
    for (int i=0; i < max_conns; i++) {
      if (conn_info_array[i].nick != NULL) {
        char msg[512];
        int offset = 0;
        strcpy(msg, "Nick: ");
        offset += 6;
        offset += insert_string(msg, conn_info_array[i].nick, offset);
        strcpy(msg+offset, "\nIP: ");
        offset += 5;
        offset += insert_string(msg, conn_info_array[i].ip, offset);
        strcpy(msg+offset, "\nPORT: ");
        offset += 7;
        insert_string(msg, conn_info_array[i].port, offset);
        msg[offset+1] = '\0';
        sprintf(msgsize, "%04lu", strlen(msg));
        send_all(sock, msgsize, 4);
        send_all(sock, msg, strlen(msg));
      }
    } 
  } else {

  }
  return 0;
}
