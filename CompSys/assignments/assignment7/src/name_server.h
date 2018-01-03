#include "csapp.h" //You can remove this if you do not wish to use the helper functions

struct socket_struct {
  int sockfd;
  int busy;
};

void reset_socket(struct socket_struct* ss) {
  ss->sockfd = -1;
  ss->busy = 0;
}

void set_socket(struct socket_struct* ss, int fd, int busy) {
  ss->sockfd = fd;
  ss->busy = busy;
}

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
