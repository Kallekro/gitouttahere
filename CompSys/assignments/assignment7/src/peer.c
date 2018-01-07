#include <stdio.h>
#include <err.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>
#include <poll.h>

#include "peer.h"
#include "socklib.h"
#include "job_queue.h"

#define ARGNUM 3

int handle_lookup(int sock, char* rec_buf, int rec_buf_len, char* buf, int buf_len);
int handle_msg(int sock, char* buf, int buflen);
int handle_show(char* buf);
int establish_connection(int* sock, char* ip, char* port); 
void* listen_handler(void* arg);
void* worker(void* arg);
int add_message(struct msg_struct* m_struct, char* nick, char* buf); 

// mutex for message array
pthread_mutex_t msg_mutex = PTHREAD_MUTEX_INITIALIZER;
// mutex for listen thread
pthread_mutex_t listen_mutex = PTHREAD_MUTEX_INITIALIZER;
 
// array to hold messages from peers
struct msg_struct* msg_array;
int msg_array_size = 100;

struct job_queue jq;

// connection info of this peer
struct connection_info my_conn_info; 

int main(int argc, char**argv) {
  if (argc != ARGNUM + 1) {
    printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
    return(0);
  }
  
  char inbuf[256]; // buffer for fgets input
  char recbuf[256]; // buffer for receive
  int size_int;
  int sockfd; // this socket is connection to name server
  int listener; // this sockets listen for peers
  
  int num_threads = atoi(argv[3]);
  if (num_threads == 0) {
    printf("invalid thread count argument");
    return 1;
  }
  
  // initialize job queue and create worker threads
  job_queue_init(&jq, 15);
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  
  for (int i=0; i<num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &jq) != 0 ) {
      err(1, "pthread_create() failed\n");
    }
  }

  // allocate memory and initialize the memory for the message array
  msg_array = malloc(sizeof(struct msg_struct)*msg_array_size);
  for (int i = 0; i < msg_array_size; i++) {
    msg_array[i].nick = NULL;
    msg_array[i].messages = malloc(10000);    
    msg_array[i].messages[0] = '\0';  
  }  
  
  int exit_flag = 0;
  while (!exit_flag) { // this loop is only exited by using /exit
    memset(recbuf, '\0', sizeof(recbuf));
    while (1) { // before login loop
      while (1) { // input loop
        printf("waiting for input..\n");
        if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
          int cmd = parsecmd(inbuf);
          if (cmd == 0) { // login
            break;
          } else if (cmd == 2) { // exit
            return 0;
          }
  
        }
      }
  
      if (establish_connection(&sockfd, argv[1], argv[2]) != 0) { 
        // if connection failed to be established go back to start of loop 
        continue;
      }
  
      // set our socket to be non blocking
      int flags = fcntl(sockfd, F_GETFL, 0);
      fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
      // send the login information from inbuf to the server
      send_msg(sockfd, inbuf, strlen(inbuf));
      // receive login response
      int ex_rec = recv_all(sockfd, recbuf, sizeof(recbuf), &size_int, "", 0); 
      if (*(recbuf+4) == '0' || ex_rec > 0) {
        // here if response from server failed. go back to start of loop and retry
        printf("%s\n", recbuf+5);
        memset(recbuf, '\0', sizeof(recbuf));
        close(sockfd);
        continue;
      }
      // here if succesfull connection
      // print the login message
      printf("%s\n", recbuf+5);
      memset(recbuf, '\0', sizeof(recbuf));
      // copy the login information into the struct to save it
      handle_login(&my_conn_info, inbuf);
      break;
    }
    // Create listener on login port.
    if (create_listener(&listener, my_conn_info.port)) {
      printf("failed to create listener\n");
      close(sockfd);
      memset(recbuf, '\0', sizeof(recbuf));
      continue; // go back and try again
    }
    
    // set the listener to be non blocking
    int flags = fcntl(listener, F_GETFL, 0);
    fcntl(listener, F_SETFL, flags | O_NONBLOCK);
  
    // create a thread to handle listening for connections
    pthread_t listen_thread;
    pthread_create(&listen_thread, NULL, &listen_handler, &listener);
  
    // non-binary flag for /logout, /exit and flag for /lookup
    int login_flag = 0, lookupflag = 0;
    // after logged in loop
    while (!login_flag) {
      if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
        switch (parsecmd(inbuf)) {
          case 0: // Login - do nothing
            printf("You are already logged in.");
            continue; 
          case 1: // Logout - logout and go to beginning of program
            login_flag = 1;
            close(sockfd);
            printf("You are now logged out\n");
            break;
          case 2: // Exit - exit program
            login_flag = 2;
            close(sockfd);
            break;
          case 3: // Lookup
            lookupflag = 1;
            break;
          case 4: // Message
            handle_msg(sockfd, inbuf, strlen(inbuf));
            continue;
          case 5: // Show
            handle_show(inbuf); // to do
            continue;
          default:
            break;
        }
        // if here some message should be sent to the server
        send_msg(sockfd, inbuf, strlen(inbuf));
  
        if (!login_flag && !lookupflag) {
          // This is probablby an unknown command being sent to the server
          if (recv_all(sockfd, recbuf, sizeof(recbuf), &size_int, "", 0) > 0) {
            printf("Server hung up");
          }
          printf("SERVER: %s\n", recbuf+4);
        }
        if (login_flag == 2) {
          // exit
          printf("Exiting program.\n");
          exit_flag = 1;
        }
  
        if (lookupflag == 1) {
          // lookup
          lookupflag = 0;
          if (handle_lookup(sockfd, recbuf, sizeof(recbuf), inbuf, strlen(inbuf)) != 0) {
            continue;
          }
        }
      }
      else {
        break;
      }
    }
    // here if logout or exit
    // lock the listener mutex and close it 
    pthread_mutex_lock(&listen_mutex);
    close(listener);
    pthread_mutex_unlock(&listen_mutex);
    // join listen thread
    if (pthread_join(listen_thread, NULL) != 0) {
      err(1, "pthread_join error\n");
    }
    // reset message array
    for (int i = 0; i < msg_array_size; i++) {
      if (!msg_array[i].nick) { continue; }
      memset(msg_array[i].nick, '\0', strlen(msg_array[i].nick));
      msg_array[i].nick = NULL;
      if (!msg_array[i].messages) { continue; }
      memset(msg_array[i].messages, '\0', 10000);
      msg_array[i].messages[0] = '\0';  
    }   
    // go to beginning of program 
  }
  // here if exit
  // Destroy jobqueue
  job_queue_destroy(&jq);
  
  // reap/join terminated threads.
  for (int i = 0; i<num_threads; i++)  {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() error\n");
    }
  }
  free(threads);
  // free the messages and the array
  for(int i = 0; i < msg_array_size; i++) {
    free(msg_array[i].messages);
  }
  free(msg_array);
  return 0;
}

// Function to establish a connection with a server or peer
int establish_connection(int* sock, char* ip, char* port) {
  // create a socket and use it to connect to the desired port and ip
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
      perror("socket");
      continue;
    }
  
    if (connect(*sock, tmp_addr->ai_addr, tmp_addr->ai_addrlen) == -1) {
      perror("connect");
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

// Function used to by the listen thread to listen for connecting peers
void* listen_handler(void* arg) {
  int new_sock;
  struct sockaddr peer_addr;
  int* listen_sock = arg;
  struct pollfd pfd;
  pfd.fd = *listen_sock;
  pfd.events = POLLIN;
  while(1) { // listen for incomming peer connections
    poll(&pfd, 1, 50);
    pthread_mutex_lock(&listen_mutex);
    unsigned int peer_addr_len = sizeof(peer_addr);
    new_sock = accept(*listen_sock, &peer_addr, &peer_addr_len);
    if (new_sock < 0) {
      if (errno == EWOULDBLOCK || errno == EAGAIN ) {
        pthread_mutex_unlock(&listen_mutex);
        continue;
      }
      else if (errno == EBADF) { 
        break;
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
  pthread_mutex_unlock(&listen_mutex);
  return NULL;
}

// worker in the thread pool
void* worker(void* arg) {
  struct job_queue *jq = arg;
  int* sock;
  char recbuf[256];
  int size_int;
  
  int spaces[1];  
  char peer_nick[100];
  
  while (1) {
    if (job_queue_pop(jq, (void**)&sock) == 0) {
      memset(recbuf, '\0', sizeof(recbuf));
      if (recv_all(*sock, recbuf, sizeof(recbuf), &size_int, "", 0) > 0) {
        perror("recieve error");
        continue;
      }
      close(*sock);
      find_spaces(recbuf+4, spaces, 1);
      strncpy(peer_nick, recbuf+4, spaces[0]);
      peer_nick[spaces[0]+1] = '\0';
      
      int first_empty = -1;
      int found_flag = 0;
      
      pthread_mutex_lock(&msg_mutex); 
      for (int i = 0; i < msg_array_size; i++) {
        if (msg_array[i].nick) {
          if (strcmp(msg_array[i].nick, peer_nick) == 0) {
            add_message(&msg_array[i], peer_nick, recbuf + 5 + spaces[0]);
            found_flag = 1;
            break;
          } 
        } else if (first_empty == -1){
          first_empty = i;
        }
      }
      if (!found_flag) {
        memset(msg_array[first_empty].messages, '\0', 10000);
        msg_array[first_empty].nick = strdup(peer_nick);
        msg_array[first_empty].offset = 0;
        add_message(&msg_array[first_empty], peer_nick, recbuf+5+spaces[0]);
      }
      pthread_mutex_unlock(&msg_mutex);
    } else {
      break;
    }
  }  
 
 
  return NULL;
}

// function for adding a new message to a msg_struct using and incrementing it's offset.
int add_message(struct msg_struct* m_struct, char* nick, char* buf) {
  strncpy(m_struct->messages + m_struct->offset, nick, strlen(nick));
  m_struct->offset += strlen(nick);
  strcpy(m_struct->messages + m_struct->offset, ": ");
  m_struct->offset += 2;
  strcpy(m_struct->messages + m_struct->offset, buf); 
  m_struct->offset += strlen(buf);
  strcpy(m_struct->messages + m_struct->offset, "\n"); 
  m_struct->offset += 1;
  return 0; 
}

// handle a lookup command
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
  } else if(extra_received > 0 ) {
    printf("recieve error or timeout\n");
    return 1;
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
    else if (extra_received > 0) {
      printf("recieve error or timeout\n");
      return 1;
    }
    strncpy(without_extra, rec_buf + 4, size_int);
    printf("%s\n\n", without_extra);
  }
  memset(rec_buf, '\0', rec_buf_len);
  
  return 0;
} 

// parse the result of a lookup request into a connection_info struct
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
  
  strncpy(_port, result + spaces[4]+1, result_len - spaces[4] - 1);
  _port[result_len - spaces[4] - 1] = '\0';
  
  ci->nick = strdup(_nick);
  ci->ip = strdup(_ip);
  ci->port = strdup(_port);
  
  return 0;
}

// handle message command      
int handle_msg(int sock, char* buf, int buflen) {
  int spaces[2];
  int space_count = find_spaces(buf, spaces, 2);
  
  if (space_count < 2) {
    printf("Wrong msg format\n");
    return 1;
  }
   
  char nick[100];
  char msg[256];
  strncpy(nick, buf+spaces[0]+1, spaces[1] - spaces[0]);
  nick[spaces[1] - spaces[0]] = '\0';
  strcpy(msg, my_conn_info.nick);
  strcpy(msg + strlen(my_conn_info.nick) , " "); 
  strcpy(msg + strlen(my_conn_info.nick)+1, buf+spaces[1]+1);
  msg[buflen - spaces[1] - 2 + strlen(my_conn_info.nick) + 1] = '\0';
  
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
  } else if (extra_received > 0) {
    printf("receive error or timeout\n");
    return 1;
  }
  
  strncpy(without_extra, rec_buf + 4, size_int);

  int conn_count = atoi(without_extra);
  struct connection_info ci;
  if (conn_count) {
    memset(rec_buf, '\0', sizeof(rec_buf));
    memset(without_extra, '\0', sizeof(without_extra));
    extra_received = recv_all(sock, rec_buf, sizeof(rec_buf), &size_int, extra_bytes, -1*extra_received);
    if (extra_received > 0) {
      printf("receive error or timeout\n");
      return 1;
    }
    strncpy(without_extra, rec_buf + 4, size_int);
    parse_lookup_result(&ci, without_extra, size_int);
    int newsock;
    establish_connection(&newsock, ci.ip, ci.port);
    send_msg(newsock, msg, strlen(msg));
    close(newsock);
  } else {
    printf("%s is not online\n", nick);
    return 0;
  }
  return 0;
}
 
// handle show commands
int handle_show(char* buf) {
  int spaces[1];
  int space_count = find_spaces(buf, spaces, 1);
  char nick[100];
  int found_flag = 0;
  pthread_mutex_lock(&msg_mutex);  
  if (space_count) {
    strcpy(nick, buf + spaces[0]+1);  
    nick[strlen(buf) - spaces[0]-2] = '\0';
    for (int i = 0; i < msg_array_size; i++) {
      if (msg_array[i].nick) {
        if (strcmp(msg_array[i].nick, nick) == 0) {
          found_flag = 1;
          printf("%s", msg_array[i].messages); 
          memset(msg_array[i].nick, '\0', strlen(msg_array[i].nick));
          msg_array[i].nick = NULL;
          break;
        }
      }
    }
    if (!found_flag) {
      printf("No new messages from %s\n", nick);
    }
  } else {
    for (int i = 0; i < msg_array_size; i++) {
      if (msg_array[i].nick) {
        found_flag = 1;
        printf("%s", msg_array[i].messages);
        memset(msg_array[i].nick, '\0', strlen(msg_array[i].nick));
        msg_array[i].nick = NULL;
      }
    }
    if (!found_flag) {
      printf("No new messages from any user.\n");
    }
    
  }
  pthread_mutex_unlock(&msg_mutex);
  return 0;
}

