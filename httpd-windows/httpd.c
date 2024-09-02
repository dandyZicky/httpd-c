#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windef.h>
#include <stdio.h>

#define LISTENING_ADDR "0.0.0.0"

char* err;

int init_srv(struct sockaddr_in* srv, int port_num) {
  SOCKET sock = INVALID_SOCKET;
  IN_ADDR ip_value;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (sock == INVALID_SOCKET) {
    err = "socket(): descriptor err\n";
    return 0;
  }

  srv->sin_family = AF_INET;

  if (!inet_pton(AF_INET, LISTENING_ADDR, &ip_value)) {
    err = "inet_pton(): network type conversion failed\n";
    return 0;
  }

  srv->sin_addr = ip_value;
  srv->sin_port = port_num;

  if (bind(sock, (struct sockaddr* ) srv, sizeof(*srv))) {
    err = "bind(): binding error";
    return 0;
  }

  if (listen(sock, 5) == SOCKET_ERROR) {
    fprintf(stderr, "listen(): Listening error\n");
    return 0;
  }
  return sock;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Usage: ./httpd <listening port>\n");
    return 0;
  }
  
  struct sockaddr_in srv, client;

  char* port;
  port = argv[1];

  WSADATA wsaData = {0};
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData);

  if (result != 0) {
    printf("WSAStartup failed: %d\n", result);
    return 0;
  }

  printf("Listening on %s:%s\n", LISTENING_ADDR, port);

  if (!init_srv(&srv, atoi(port))){
    fprintf(stderr, "%s\n", err);
    return 0;
  }

  return EXIT_SUCCESS;

}
