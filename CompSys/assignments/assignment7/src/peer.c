#include <stdio.h>
#include <err.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>

#include "peer.h"
#include "socklib.h"
#include "job_queue.h"

#define ARGNUM 3 // TODO: Put the number of you want to take

#define DEFAULT_NAME "localhost"
#define DEFAULT_PORT "50000"
  
pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t listen_mutex = PTHREAD_MUTEX_INITIALIZER;

int handle_lookup(int sock, char* rec_buf, int rec_buf_len, char* buf, int buf_len);
int handle_msg(int sock, char* buf, int buflen);
int establish_connection(int* sock, char* ip, char* port); 
void* listen_handler(void* arg);
void* worker(void* arg);

struct msg_struct* msg_array;
struct job_queue jq;

int main(int argc, char**argv) {
  if (argc != ARGNUM + 1) {
    printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
    return(0);
  }
  
  char inbuf[256];
  char recbuf[256];
  int size_int;
  memset(recbuf, '\0', sizeof(recbuf));
  int sockfd;
  int listener;

  struct connection_info my_conn_info; 

  int num_threads = atoi(argv[3]);
  if (num_threads == 0) {
    printf("invalid thread count argument");
    return 1;
  }

  job_queue_init(&jq, 15);
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));

  for (int i=0; i<num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &jq) != 0 ) {
      err(1, "pthread_create() failed\n");
    }
  }
  msg_array = malloc(sizeof(struct msg_struct)*100);

  while (1) { // client super loop
    while (1) { // before login loop
      while (1) { // input loop
        printf("Please give input\n");
        if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
          int cmd = parsecmd(inbuf);
          if (cmd == 0) { // login
            break;
          } else if (cmd == 2) { // exit
            return 0;
          }

        }
      }

      if (establish_connection(&sockfd, argv[1], argv[2])) {
        continue;
      }

      int flags = fcntl(sockfd, F_GETFL, 0);
      fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

      send_msg(sockfd, inbuf, strlen(inbuf));

      recv_all(sockfd, recbuf, sizeof(recbuf), &size_int, "", 0); 
      if (*(recbuf+4) == '0') {
        printf("%s\n", recbuf+5);
        memset(recbuf, '\0', sizeof(recbuf));
        close(sockfd);
        continue;
      }
      printf("%s\n", recbuf+5);
      memset(recbuf, '\0', sizeof(recbuf));
      handle_login(&my_conn_info, inbuf);
      break;
    }
    // Create thread with listener on login port.
    if (create_listener(&listener, my_conn_info.port)) {
      fprintf(stderr, "failed to create listener");
      continue;
    }

    int flags = fcntl(listener, F_GETFL, 0);
    fcntl(listener, F_SETFL, flags | O_NONBLOCK);

    pthread_t listen_thread;
    pthread_create(&listen_thread, NULL, &listen_handler, &listener);

    int login_flag = 0, lookupflag = 0, msgflag = 0;
    while (!login_flag) {
      if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
        switch (parsecmd(inbuf)) {
        case 0: // Login
          printf("You are already logged in.");
          continue; 
          case 1: // Logout
            login_flag = 1;
            close(sockfd);
            printf("You are now logged out\n");
            break;
          case 2: // Exit
            login_flag = 2;
            close(sockfd);
            break;
          case 3: // Lookup
            lookupflag = 1;
            break;
          case 4: // MSG
            msgflag = 1;
            break;
          case 5: // SHOW
            break;
          default:
            break;
        }
        if (msgflag) {
          msgflag = 0;
          handle_msg(sockfd, inbuf, strlen(inbuf));
          continue;
        }

        send_msg(sockfd, inbuf, strlen(inbuf));

        if (!login_flag && !lookupflag) {
          if (recv_all(sockfd, recbuf, sizeof(recbuf), &size_int, "", 0) == -1) {
            printf("Server hung up");
          }
          printf("SERVER: %s\n", recbuf+4);
        }
        if (login_flag == 2) {
          return 0;
        }

        if (lookupflag == 1) {
          lookupflag = 0;
          if (handle_lookup(sockfd, recbuf, sizeof(recbuf), inbuf, strlen(inbuf))) {
            continue;
          }
        }
      }
      else {
        break;
      }
    }
    pthread_mutex_lock(&listen_mutex);
    close(listener);
    pthread_mutex_unlock(&listen_mutex);

    if (pthread_join(listen_thread, NULL) != 0) {
      err(1, "pthread_join error\n");
    }
  }

  // Destroy jobqueue
  job_queue_destroy(&jq);

  // reap/join terminated threads.
  for (int i = 0; i<num_threads; i++)  {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() error\n");
    }
  }
  free(threads);

  return 0;
}

int establish_connection(int* sock, char* ip, char* port) {
  struct addrinfo hints, *addri_res, *tmp_addr;
  int addr_err;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if ((addr_err = getaddrinfo(ip, port, &hints, &addri_res)) != 0) {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(addr_err));
    return 1;
  }

  for (tmp_addr = addri_res; tmp_addr != NULL; tmp_addr = tmp_addr->ai_next) {
    if ((*sock = socket(tmp_addr->ai_family, tmp_addr->ai_socktype, tmp_addr->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(*sock, tmp_addr->ai_addr, tmp_addr->ai_addrlen) == -1) {
      perror("client: connect");
      continue;
    }
    break;
  }
  if (tmp_addr == NULL) {
    fprintf(stderr, "Failed to connect\n");
    return 2;
  }
  freeaddrinfo(addri_res);

  return 0;
}

void* listen_handler(void* arg) {
  int new_sock;
  struct sockaddr peer_addr;
  int* listen_sock = arg;
  while(1) { // listen for incomming peer connections
    pthread_mutex_lock(&listen_mutex);
    unsigned int peer_addr_len = sizeof(peer_addr);
    if ((new_sock = (accept(*listen_sock, &peer_addr, &peer_addr_len)) < 0)) {
      if (errno == EWOULDBLOCK || errno == EAGAIN ) {
        pthread_mutex_unlock(&listen_mutex);
        continue;
      } else {
        perror("accept");
        break;
      }
    }
    int flags = fcntl(new_sock, F_GETFL, 0);
    fcntl(new_sock, F_SETFL, flags | O_NONBLOCK);

    job_queue_push(&jq, &new_sock);
    pthread_mutex_unlock(&listen_mutex);
  }
  return NULL;
}

void* worker(void* arg) {
  struct job_queue *jq = arg;
  int* sock;
  char recbuf[256];
  int size_int;

  while (1) {
    if (job_queue_pop(jq, (void**)&sock) == 0 ) {
      memset(recbuf, '\0', sizeof(recbuf));
      recv_all(*sock, recbuf, sizeof(recbuf), &size_int, "", 0);
      printf("Recbuf: %s\n", recbuf);
      close(*sock);
    }
  }

  return NULL;
}


int handle_lookup(int sock, char* rec_buf, int rec_buf_len, char* buf, int buf_len) {
  int extra_received;
  char extra_bytes[256];
  char without_extra[256];
  int size_int;
  int single_lookup = 0;
  int spaces[1];
  int spacecount = find_spaces(buf, spaces, 1);
  char target_nick[100];
  if (spacecount > 0) {
    strncpy(target_nick, buf + spaces[0]+1, buf_len - spaces[0]);
    target_nick[buf_len- spaces[0]-2] = '\0';
    if (buf_len - spaces[0]-2 > 0) {
      single_lookup = 1;
    }
  }
  
  extra_received = recv_all(sock, rec_buf, rec_buf_len, &size_int, "", 0);
  if (extra_received < 0) { // extra bytes is -1 for each extra byte
    strcpy(extra_bytes, rec_buf + 4 + size_int);
  }
  strncpy(without_extra, rec_buf + 4, size_int);

  int conn_count = atoi(without_extra);
  if (!conn_count && !single_lookup) {
    printf("atoi error\n");
    return 1;
  } else if (!conn_count && single_lookup) {
    printf("%s is not online\n", target_nick);
    return 1;
  } else if (conn_count && single_lookup) {
    printf("%s is online\n", target_nick);
  } else {
    printf("%d users online. The list follows\n", conn_count);
  }
  for (int i=0; i<conn_count; i++) {
    memset(rec_buf, '\0', rec_buf_len);
    memset(without_extra, '\0', sizeof(without_extra));
    extra_received = recv_all(sock, rec_buf, rec_buf_len, &size_int, extra_bytes, -1*extra_received);
    memset(extra_bytes, '\0', sizeof(extra_bytes));
    if (extra_received < 0) { // extra bytes is -1 for each extra byte
      strcpy(extra_bytes, rec_buf + 4 + size_int);
    }
    strncpy(without_extra, rec_buf + 4, size_int);
    printf("%s\n\n", without_extra);
  }
  memset(rec_buf, '\0', rec_buf_len);
  
  return 0;
} 

int parse_lookup_result (struct connection_info* ci, char* result, int result_len) {
  int spaces[5];
  if (find_spaces(result, spaces, 5) != 5) {
    printf("Incorrect lookup result\n");
    return 1;
  }

  char _nick[100];
  char _ip[100];
  char _port[100];
  strncpy(_nick, result + spaces[0]+1, spaces[1] - spaces[0] - 1);
  _nick[spaces[1] - spaces[0]-1] = '\0';

  strncpy(_ip, result + spaces[2]+1, spaces[3] - spaces[2] - 1);
  _ip[spaces[3] - spaces[2]-1] = '\0';

  strncpy(_port, result + spaces[4]+1, result_len - spaces[4] - 2);
  _port[result_len - spaces[4] - 2] = '\0';

  ci->nick = strdup(_nick);
  ci->ip = strdup(_ip);
  ci->port = strdup(_port);

  return 0;
}

int handle_msg(int sock, char* buf, int buflen) {
  int spaces[2];
  find_spaces(buf, spaces, 2);

  char nick[100];
  char msg[256];
  strncpy(nick, buf+spaces[0]+1, spaces[1] - spaces[0]);
  nick[spaces[1] - spaces[0]] = '\0';
  strcpy(msg, buf+spaces[1]+1);
  msg[buflen - spaces[1] - 2] = '\0';

  char lookup_query[100];
  sprintf(lookup_query, "/lookup %s%c", nick, '\0');
  send_msg(sock, lookup_query, strlen(lookup_query));

  int extra_received;
  char extra_bytes[256];
  char without_extra[256];
  int size_int;
  char rec_buf[256];

  extra_received = recv_all(sock, rec_buf, sizeof(rec_buf), &size_int, "", 0);
  if (extra_received < 0) { // extra bytes is -1 for each extra byte
    strcpy(extra_bytes, rec_buf + 4 + size_int);
  }
  strncpy(without_extra, rec_buf + 4, size_int);

  int conn_count = atoi(without_extra);
  struct connection_info ci;
  if (conn_count) {
    memset(rec_buf, '\0', sizeof(rec_buf));
    memset(without_extra, '\0', sizeof(without_extra));
    extra_received = recv_all(sock, rec_buf, sizeof(rec_buf), &size_int, extra_bytes, -1*extra_received);
    strncpy(without_extra, rec_buf + 4, size_int);
    parse_lookup_result(&ci, without_extra, size_int);
    int newsock;
    printf("%s, %s\n", ci.ip, ci.port);
    establish_connection(&newsock, ci.ip, ci.port);
    send_msg(newsock, msg, strlen(msg));
    close(newsock);
  } else {
    printf("%s is not online", nick);
    return 0;
  }
  return 0;
}

