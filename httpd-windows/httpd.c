#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string.h>
#include <windef.h>
#include <stdio.h>

#define LISTENING_ADDR "127.0.0.1"
#define BUF_LEN_DEFAULT 512
#define BUF_LEN_LARGE 1024 

char* err;

/*** Is there an easy way to see the docs in Windows like linux's cli man pages? ***/

int init_srv(struct sockaddr_in* srv, int port_num) {
  SOCKET sock = INVALID_SOCKET;
  // IN_ADDR ip_value;

  sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (sock == INVALID_SOCKET) {
    err = "socket(): descriptor err\n";
    return 0;
  }

  srv->sin_family = AF_INET;
  srv->sin_port = htons(port_num);

  if (!inet_pton(AF_INET, LISTENING_ADDR, &srv->sin_addr)) {  // inet_addr() is deprecated
    err = "inet_pton(): network type conversion failed\n";    // I want to know what is assigned
    return 0;                                                 // to ip_value
  }

  if (bind(sock, (struct sockaddr* ) srv, sizeof(*srv))) {
    err = "bind(): binding error";
    return 0;
  }

  if (listen(sock, 8) == SOCKET_ERROR) {
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
  int result = WSAStartup(MAKEWORD(2, 2), &wsaData); // Windows Socket API (WSA duh)

  if (result != 0) {
    printf("WSAStartup failed: %d\n", result);
    return 0;
  }

  printf("Listening on %s:%s\n", LISTENING_ADDR, port);
  SOCKET sock = init_srv(&srv, atoi(port));
  if (!sock){
    fprintf(stderr, "%s\n", err);
    return 0;
  }

  int client_length = sizeof(struct sockaddr_in);
  SOCKET accepting_client = accept(sock, (struct sockaddr*) &client, &client_length);
  // SOCKET accepting_client = accept(sock, NULL, NULL);

  if (accepting_client == INVALID_SOCKET) {
    fprintf(stderr, "Error accepting client: %d\n", WSAGetLastError());
    return 0;
  } else {
    printf("Client accepted!\n");

    // Appeared to be deprecated
    // printf("Client: %s", inet_ntoa(client.sin_addr/

    char pAddr [15]; 
    inet_ntop(AF_INET, &client.sin_addr, pAddr, sizeof(pAddr));
    printf("Client: %s\n", pAddr);
  }

  /*  
   * Congrats, to me, by me. Longest hello world to implement
   * TODO: how do you map certain routes and request?
   *       are you even sure the current code is what it is suppose to be like
   *       when returning a 200 containing basic HTML?
   */

  char recv_buf[BUF_LEN_LARGE];
  int recv_buf_len = BUF_LEN_LARGE;

  const char* headers =
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html\r\n"
      "Connection: close\r\n"
      "\r\n";
  
  const char* body = "<html><body><h1>Hello, World!</h1></body></html>";
  
  char reply[BUF_LEN_LARGE];
  snprintf(reply, sizeof(reply), "%s%s", headers, body);
  
  int receive_result;
  do {
      if ((receive_result = recv(accepting_client, recv_buf, recv_buf_len, 0)) > 0) {
          printf("Bytes received: %d\n", receive_result);
          printf("Message: %s\n", recv_buf);
          
          int send_result = send(accepting_client, reply, strlen(reply), 0);
          if (send_result == SOCKET_ERROR) {
              fprintf(stderr, "Send failed with error: %d\n", WSAGetLastError());
          } else {
              printf("Reply sent\n");
              closesocket(accepting_client); // The part where i'm on the fence about
          }
      } else if (receive_result == 0) {
          printf("Socket is closing...\n");
      } else {
          fprintf(stderr, "Error receiving buffer: %d\n", receive_result);
          closesocket(sock);
          closesocket(accepting_client);
          WSACleanup();
          return 0;
      }
    printf("Receive Result State: %d\n", receive_result);
  } while (receive_result > 0);
  
  closesocket(sock);
  closesocket(accepting_client);
  WSACleanup();
  printf("sockets closed \n");
  return EXIT_SUCCESS;
}
