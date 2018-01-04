#include <stdio.h>
#include "peer.h"
#include <string.h>
#include "socklib.h"

#define ARGNUM 2 // TODO: Put the number of you want to take

#define DEFAULT_NAME "localhost"
#define DEFAULT_PORT "50000"

const char* cmdlist[4] = { "/login", "/logout", "/exit", "/lookup" };
const int cmdlen = 4;
int parsecmd(char* input);

int main(int argc, char**argv) {
  if (argc != ARGNUM + 1) {
    printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
    return(0);
  }
  
  char inbuf[256];
  char recbuf[256];
  int recvbytes;
  memset(recbuf, '\0', sizeof(recbuf));
  char msgsize[4];
  int sockfd;
  struct addrinfo hints, *addri_res, *tmp_addr;
  int addr_err;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
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
  
      if ((addr_err = getaddrinfo(argv[1], argv[2], &hints, &addri_res)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(addr_err));
        exit(1);
      } 
      
      for (tmp_addr = addri_res; tmp_addr != NULL; tmp_addr = tmp_addr->ai_next) {
        if ((sockfd = socket(tmp_addr->ai_family, tmp_addr->ai_socktype, tmp_addr->ai_protocol)) == -1) {
          perror("client: socket");
          continue;
        }
  
        if (connect(sockfd, tmp_addr->ai_addr, tmp_addr->ai_addrlen) == -1) {
          perror("client: connect");
          continue;
        }
        break;
      }
      if (tmp_addr == NULL) {
        fprintf(stderr, "Failed to connect\n");
        exit(5);
      }
      freeaddrinfo(addri_res);
  
      int flags = fcntl(sockfd, F_GETFL, 0);
      fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

  
      sprintf(msgsize, "%04lu", strlen(inbuf));
      send_all(sockfd, msgsize, 4);
      send_all(sockfd, inbuf, strlen(inbuf));
      
      recv_all(sockfd, recbuf, sizeof(recbuf), &recvbytes, "", 0); //recv(sockfd, recbuf, sizeof(recbuf), 0);
      if (*(recbuf+4) == '0') {
        printf("%s\n", recbuf+5);
        memset(recbuf, '\0', sizeof(recbuf));
        close(sockfd);
        continue;
      }
      printf("%s\n", recbuf+5);
      memset(recbuf, '\0', sizeof(recbuf));
      break;
    }
    int loopflag = 0, lookup = 0;
    while (!loopflag) {
      if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
        switch (parsecmd(inbuf)) {
          case 0: // Login
            printf("You are already logged in.");
            continue; 
          case 1: // Logout
            loopflag = 1;
            close(sockfd);
            printf("You are now logged out\n");
            break;
          case 2: // Exit
            loopflag = 2;
            close(sockfd);
            break;
          case 3: // Lookup
            lookup = 1;
            break;
          default:
            break;
        }
        sprintf(msgsize, "%04lu", strlen(inbuf));
        send_all(sockfd, msgsize, 4);
        send_all(sockfd, inbuf, strlen(inbuf));

        if (!loopflag && !lookup) {
          if (recv_all(sockfd, recbuf, sizeof(recbuf), &recvbytes, "", 0) == -1) {
            printf("Server hung up");
          }
          printf("Received from server: %s\n", recbuf+4);
        }
        if (loopflag == 2) {
          return 0;
        }

        int extra_received;
        char extra_bytes[256];
        char without_extra[256];
        if (lookup == 1) {
          lookup = 0;

          extra_received = recv_all(sockfd, recbuf, sizeof(recbuf), &recvbytes, "", 0);
          if (extra_received < 0) { // extra bytes is -1 for each extra byte
            strcpy(extra_bytes, recbuf + 4 + recvbytes);
          }
          strncpy(without_extra, recbuf + 4, recvbytes);
          //printf("without: %s\n", without_extra);
          int conn_count = atoi(without_extra);
          if (conn_count == 0) {
            printf("atoi error\n");
            continue;
          }
          printf("%d users online. The list follows\n", conn_count);
          for (int i=0; i<conn_count; i++) {
            memset(recbuf, '\0', sizeof(recbuf));
            memset(without_extra, '\0', sizeof(without_extra));
            extra_received = recv_all(sockfd, recbuf, sizeof(recbuf), &recvbytes, extra_bytes, -1*extra_received);
            memset(extra_bytes, '\0', sizeof(extra_bytes));
            if (extra_received < 0) { // extra bytes is -1 for each extra byte
              strcpy(extra_bytes, recbuf + 4 + recvbytes);
              //printf("recvbytes: %d\n", recvbytes);
              //printf("extra: %d\n", extra_received);
              //printf("bytes: %s\n", extra_bytes);
            }
            strncpy(without_extra, recbuf + 4, recvbytes);
            printf("%s\n\n", without_extra);
          }
        }
      }
      else {
        break;
      }
    }
  }
  return 0;
}

// Parse which command is in input
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

