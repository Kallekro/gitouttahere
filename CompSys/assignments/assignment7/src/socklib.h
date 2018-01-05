#ifndef SOCK_LIB
#define SOCK_LIB
#include <sys/time.h>

// Function to receive all data on a non-blocking socket
// Can take extra data (extrabytes argument). We need this because
// we call a receive for each send but one receive could get the data from 2 sends, so the extra
// data is carried on to the next receive.
int recv_all(int sock, char* buf, int buflen, int* recbytes, char* extrabytes, int extra_len) {
  struct timeval begin, now;
  gettimeofday(&begin, NULL);
  begin.tv_sec += 5;
  int bytesreceived = 0;
  int initbytes = 4;
  if (extra_len > 0) {
    strcpy(buf, extrabytes);
    bytesreceived = extra_len;
  }
  while (bytesreceived < initbytes) {
    gettimeofday(&now, NULL);
    if (now.tv_sec * 1000000 + now.tv_usec > begin.tv_sec * 1000000 + begin.tv_usec) {
      printf("receive timed out\n");
      return 2;
    }
    int received = recv(sock, buf+bytesreceived, buflen, 0);
    if (received > 0) {
      bytesreceived += received;
    } else if (received == 0) {
      // some error (server hung)
      return -1;
    }
  }
  char size[4];
  strncpy(size, buf, 4);
  int size_int = atoi(size);
  int bytesremain = size_int - (bytesreceived - 4);
  while (bytesremain > 0) {
    gettimeofday(&now, NULL);
    if (now.tv_sec * 1000000 + now.tv_usec > begin.tv_sec * 1000000 + begin.tv_usec) {
      printf("receive timed out\n");
      return 2;
    }
    int received = recv(sock, buf+bytesreceived, buflen, 0);
    if (received > 0) {
      bytesremain -= received;
      bytesreceived += received;
    }
  }
  *recbytes = size_int;
  return bytesremain;
}

// Function to send all data on a non-blocking socket
int send_all(int sock, char* data, int datalen) {
  int total_bytes = 0;
  while (total_bytes < datalen) {
    int sent_bytes = send(sock, data+total_bytes, datalen-total_bytes, 0);
    if (sent_bytes == -1) {
      return -1;
    }
    total_bytes += sent_bytes;
  }
  return total_bytes;
}

// Function we use to send messages. 
// It starts by sending the length of the next message in the format XXXX, eg. 0023 = 23 bytes
// This way the other end knows how much data it needs to read.
int send_msg(int sock, char* msg, size_t msglen) {
  char msglen_str[4];
  sprintf(msglen_str, "%04lu", msglen);
  send_all(sock, msglen_str, 4);
  send_all(sock, msg, msglen);
  return 0;
}

const char* cmdlist[4] = { "/login", "/logout", "/exit", "/lookup" };
const int cmdlen = 4;

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

#endif
