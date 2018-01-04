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
  char msgsize[4];
  int sockfd;
  struct addrinfo hints, *addri_res, *tmp_addr;
  int addr_err;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  while (1) {
    while (1) {
      printf("Please give input\n");
      if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
        if (parsecmd(inbuf) == 0) {
          break;
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
    
    recv_all(sockfd, recbuf, sizeof(recbuf)); //recv(sockfd, recbuf, sizeof(recbuf), 0);
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
  
  while (1) {
    printf("HEEEY\n");
    if (fgets(inbuf, sizeof(inbuf), stdin) != NULL) {
      sprintf(msgsize, "%04lu", strlen(inbuf));
      send_all(sockfd, msgsize, 4);
      int sent = send_all(sockfd, inbuf, strlen(inbuf));
      printf("%d\n", sent);
      int rec_return;
      if ((rec_return = recv_all(sockfd, recbuf, sizeof(recbuf))) == -1) {
        printf("Server hung up");
      }
      printf("Received from server: %s\n", recbuf+4);
      
    }
    else {
      break;
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

