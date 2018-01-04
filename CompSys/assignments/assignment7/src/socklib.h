#ifndef SOCK_LIB
#define SOCK_LIB
#include <sys/time.h>

// Function to receive all data on a non-blocking socket
int recv_all(int sock, char* buf, int buflen) {
  struct timeval begin, now;
  gettimeofday(&begin, NULL);
  begin.tv_sec += 5;
  int bytesreceived = 0;
  int initbytes = 4;
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
  int bytesremain = atoi(size) - (bytesreceived - 4);
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
  return bytesreceived;
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


#endif
