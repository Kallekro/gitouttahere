#ifndef SOCK_LIB
#define SOCK_LIB
#include <sys/time.h>

struct connection_info {
  int sockfd;
  char* nick;
  char* passwd;
  char* ip;
  char* port;
};

void set_connection_info(struct connection_info* ci, char* nick, char* passwd, char* ip, char* port) {
  ci->nick = nick;
  ci->passwd = passwd;
  ci->ip = ip;
  ci->port = port;
}

int create_listener(int* listener_sock, char* port) {

  struct addrinfo hints, *addri_res, *tmp_addr;
  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int addr_err, sock_flag = 1;
  if ((addr_err = getaddrinfo(NULL, port, &hints, &addri_res)) != 0) {
    fprintf(stderr, "server: %s\n", gai_strerror(addr_err));
    return 1;
  }

  for (tmp_addr = addri_res; tmp_addr != NULL; tmp_addr = tmp_addr->ai_next) {
    *listener_sock = socket(tmp_addr->ai_family, tmp_addr->ai_socktype, tmp_addr->ai_protocol);

    if (*listener_sock < 0) {
      continue;
    }

    setsockopt(*listener_sock, SOL_SOCKET, SO_REUSEADDR, &sock_flag, sizeof(int));

    if (bind(*listener_sock, tmp_addr->ai_addr, tmp_addr->ai_addrlen) < 0)  {
      close(*listener_sock);
      continue;
    }
    break;
  }

  if (tmp_addr == NULL) {
    fprintf(stderr, "Failed to bind to socket\n");

    return 1;
  }

  freeaddrinfo(addri_res);

  if(listen(*listener_sock, 15) < 0 ) {
    fprintf(stderr, "Failed to listen on socket\n");
    return 1;
  }
  return 0; 
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
  char _nick[100];
  char _passwd[100];
  char _ip[100];
  char _port[100];
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
  return 0;
}

// Function to receive all data on a non-blocking socket
// Can take extra data (extrabytes argument). We need this because
// we call a receive for each send but one receive could get the data from 2 sends, so the extra
// data is carried on to the next receive.
int recv_all(int sock, char* buf, int buflen, int* size_int, char* extrabytes, int extra_len) {
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
      // timed out
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
  *size_int = atoi(size);
  int bytesremain = *size_int - (bytesreceived - 4);
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

const char* cmdlist[6] = { "/login", "/logout", "/exit", "/lookup", "/msg", "/show" };
const int cmdlen = 6;

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

#endif
