#include <stdio.h>
#include <err.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdbool.h>

#include "name_server.h"
#include "job_queue.h"
#include "socklib.h"
#define ARGNUM 2

// master mutex for the fd_set master_set
pthread_mutex_t master_mutex = PTHREAD_MUTEX_INITIALIZER;
// connection_info_array mutex
pthread_mutex_t conn_info_mutex = PTHREAD_MUTEX_INITIALIZER;

// a set for containing connected sockets
fd_set master_set;
// tmp set used by select
fd_set tmp_set;

// struct array holding login information of connecting peer
struct connection_info* conn_info_array;
int conn_count;

int max_conns = 100; // maximum number of connections

int listener; // listener socket
bool listening = false;
int max_fd; // highest socket-fd (for looping through the sets

void* input_handler(); // function for handling input to server
int shutdown_flag = 0;

void* worker(void* arg);
int handle_lookup(int sock, char* input, int inputlen);

int main(int argc, char**argv) {
  if (argc != ARGNUM + 1) {
    printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
    return(0);
  }


  int num_threads = atoi(argv[1]);
  if (num_threads == 0) {
    printf("invalid thread count argument");
    return 1;
  }
    
  char* PORT = argv[2];
  
  printf("Starting server on port %s..\n", PORT);

  if (create_listener(&listener, PORT)) {
    exit(2);
  }
  
  // clears the set
  FD_ZERO(&master_set);
  FD_ZERO(&tmp_set);
  
  // add listener to master set
  FD_SET(listener, &master_set);
  
  max_fd = listener; // listener is current highest fd

  // allocating space for connection info array
  int conn_array_len = max_conns; 
  conn_info_array = malloc(conn_array_len * sizeof(struct connection_info));
  conn_count = 0; 
  // initialize structs in array
  for (int i=0; i<max_conns; i++) {
    set_connection_info(&conn_info_array[i], NULL, NULL, NULL, NULL);
  }

  // initialize jobqueue
  struct job_queue jq;
  job_queue_init(&jq, 15);
  // allocate space for threads
  pthread_t *threads = calloc(num_threads, sizeof(pthread_t));
  
  // create worker thread-pool
  for (int i=0; i<num_threads; i++) {
    if (pthread_create(&threads[i], NULL, &worker, &jq) != 0 ) {
      err(1, "pthread_create() failed\n");
    }
  }

  pthread_t input_thread;
  pthread_create(&input_thread, NULL, &input_handler, NULL);

  struct sockaddr client_addr; // struct holding information about clients address
  int new_sock; // socket for incomming connections
  // main loop
  while (1) {
    pthread_mutex_lock(&master_mutex);
    if (shutdown_flag) {
      pthread_mutex_unlock(&master_mutex);
      break;
    }
    tmp_set = master_set; // copy the master set
    pthread_mutex_unlock(&master_mutex);
    struct timeval timeout;
    timeout.tv_usec = 100; // set timeout
    timeout.tv_sec = 0;
    // Use select to obtain sockets that are ready.
    if (select(max_fd+1, &tmp_set, NULL, NULL, &timeout) == -1) {
      perror("select");
      exit(1);
    }

    for (int i=0; i <= max_fd; i++) { // iterate over sockets connections
      if (FD_ISSET(i, &tmp_set)) {    // if the socket is ready
        if (i == listener) { // if the socket is the listener - theres a new connection
          unsigned int addr_len = sizeof(client_addr);
          new_sock = accept(listener, &client_addr, &addr_len); // accept new connection
          printf("accepted new connection\n");
          if (new_sock == -1) {
            perror("accept");
          }
        } else { // If not listener, a connected socket has incomming data.
          pthread_mutex_lock(&master_mutex);
          new_sock = i;
          FD_CLR(i, &master_set);  
          pthread_mutex_unlock(&master_mutex);
        }
        // add the readable socket to the job queue
        job_queue_push(&jq, &new_sock);
      }
    }
  }

  if (pthread_join(input_thread, NULL) != 0) {
    err(1, "pthread_join error\n");
  }

  // Destroy jobqueue
  job_queue_destroy(&jq);

  // reap/join terminated threads.
  for (int i = 0; i<num_threads; i++)  {
    if (pthread_join(threads[i], NULL) != 0) {
      err(1, "pthread_join() error\n");
    }
  }
  free(threads); // free threads
  close(listener); 

  for (int i = 0; i < max_conns; i++) {
    if (conn_info_array[i].nick) {
      close(i); // close connection
    }
  }
  free(conn_info_array);

  return 0;
}

void* input_handler() {
  while(1) {
    char inbuf[100];
    printf("Type /shutdown to shut down server.\n");
    if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
      inbuf[strlen(inbuf)-1] = '\0';
      if (strcmp(inbuf, "/shutdown") == 0) {
        pthread_mutex_lock(&master_mutex);
        shutdown_flag = 1;
        pthread_mutex_unlock(&master_mutex);
        printf("Shutting down server.\n");
        break;
      } 
    }
  }
  return NULL;
}

void* worker(void * arg) { 
  struct job_queue *jq = arg; // get the jobqueue


  char data_buf[512];  // recieving data buffer
  int recvbytes;      
  
  int* sock;
  while(1) {
    if (job_queue_pop(jq, (void**)&sock) == 0) { // pop a ready socket
      memset(data_buf, '\0', sizeof(data_buf)); // ensure clear memory
      pthread_mutex_lock(&conn_info_mutex);      
      if (!conn_info_array[*sock].nick) {  // if the sending socket is not connected - is probably a login.
        // A new connection
        int flags = fcntl(*sock, F_GETFL, 0);
        fcntl(*sock, F_SETFL, flags | O_NONBLOCK); // set socket to nonblocking

        if (recv_all(*sock, data_buf, sizeof(data_buf), &recvbytes, "", 0) > 0) { // recieve message from socket
          printf("receive error or timeout\n");
          printf("closed connection.\n");
          close(*sock); // close the socket.
          continue;
        }
        
        // TODO: CHECK IF CORRECT LOGIN

        char loginmsg[100];
        if (handle_login(&(conn_info_array[*sock]), data_buf+4) == 0) {
          // If the login was correct - construct login message to respond with.
          strncpy(loginmsg, "1Login succesful. Welcome \0", 30);
          strncpy(loginmsg + strlen(loginmsg), conn_info_array[*sock].nick, 50);
          // send welcoming message to client
          send_msg(*sock, loginmsg, strlen(loginmsg));
          conn_count += 1; // we a one more connection
        } else { // else login was unsuccessfull
          sprintf(loginmsg, "0Login unsuccesful. Please check you login information and try again\n");
          send_msg(*sock, loginmsg, strlen(loginmsg));
          close(*sock); // close connection
          printf("closed connection\n");
          pthread_mutex_unlock(&conn_info_mutex);
          continue; // pop a new job.
        }

      pthread_mutex_unlock(&conn_info_mutex);
      

      pthread_mutex_lock(&master_mutex); /// locking mutex
      // add the new connection to master set
      FD_SET(*sock, &master_set);
      if (*sock > max_fd) {
        max_fd = *sock; // maintain highest fd
      }
      pthread_mutex_unlock(&master_mutex); /// unlocking mutex

      } else { // if a already logged in client is sending a command
        pthread_mutex_unlock(&conn_info_mutex);
        // receive incoming command
        if (recv_all(*sock, data_buf, sizeof(data_buf), &recvbytes, "", 0) > 0) {
          conn_info_array[*sock].nick = NULL;
          close(*sock);
          conn_count--;
          printf("closed connection\n");
          continue;
        }
        char msg[100]; // respond message
        int remove_sock = 0;
        /// Handle recived command.
        switch (parsecmd(data_buf+4)) {
          case 0: // Login
            sprintf(msg, "You are already logged in.");
            send_msg(*sock, msg, strlen(msg));
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
            handle_lookup(*sock, data_buf+4,(int)strlen(data_buf+4));
            pthread_mutex_unlock(&conn_info_mutex);
            break;
          default:
            sprintf(msg, "Unknown command.");
            send_msg(*sock, msg, strlen(msg));
            break;
        }
        if (!remove_sock) {
          pthread_mutex_lock(&master_mutex);
          FD_SET(*sock, &master_set);
          pthread_mutex_unlock(&master_mutex);
        }
      }
    } else { // Jobqueue was killed
      break;
    }
  } // end worker while
  return NULL;
}


int insert_string(char* dst, char* src, int offset) {
  strcpy(dst+offset, src);
  return strlen(src);
}


int construct_lookup_msg(char* msgbuf, char* nick, char* ip, char* port) {
  // This function constructs a lookup respond given a nickname, ip and port.
  int offset = 0;
  strcpy(msgbuf, "Nick: ");
  offset += 6;
  offset += insert_string(msgbuf, nick, offset);
  strcpy(msgbuf+offset, " \nIP: ");
  offset += 6;
  offset += insert_string(msgbuf, ip, offset);
  strcpy(msgbuf+offset, " \nPORT: ");
  offset += 8;
  offset += insert_string(msgbuf, port, offset);
  msgbuf[offset+1] = '\0';
  return 0;
}

int handle_lookup(int sock, char* input, int inputlen) {
  // This function responds to a lookup-request from a client
  // by searcing through the conn_info_array and responding properly.
  int spaces[1];
  spaces[0] = 0;
  char msg[512];
  char conn_count_str[4];
  int spacecount = find_spaces(input, spaces, 1);
  if ( spacecount < 1 || inputlen - spaces[0]-2 < 1) { // lookup without nickname
    sprintf(conn_count_str, "%d", conn_count);
    send_all(sock, "0004", 4); // send to the client with ammount of incomming data
    send_all(sock, conn_count_str, 4);
    for (int i=0; i < max_conns; i++) { // Construct lookup respond with construct_lookup_msg
      if (conn_info_array[i].nick != NULL) {
        construct_lookup_msg(msg, conn_info_array[i].nick, conn_info_array[i].ip, conn_info_array[i].port);
        send_msg(sock, msg, strlen(msg));
      }
    } 
  } else { // If lookup with specific nickname
    // nickname to look up
    char target_nick[100]; 
    strncpy(target_nick, input + spaces[0]+1, inputlen - spaces[0]-1);
    target_nick[inputlen - spaces[0]-2] = '\0';
    for (int i=0; i < max_conns; i++) {
      if (!conn_info_array[i].nick) { continue; } // if NULL continue
      if (strcmp(conn_info_array[i].nick, target_nick) == 0) { // If the user is online
        sprintf(conn_count_str, "%d", 1);
        send_all(sock, "0004", 4);
        send_all(sock, conn_count_str, 4);
        // Construct lookup respond
        construct_lookup_msg(msg, conn_info_array[i].nick, conn_info_array[i].ip, conn_info_array[i].port);
        // send lookup respond
        send_msg(sock, msg, strlen(msg));
        return 0;
      }
    }
    // if here target was not online
    sprintf(conn_count_str, "%d", 0);
    send_all(sock, "0004", 4);
    send_all(sock, conn_count_str, 4);
  }
  return 0;
}
