/*
 *
 * Copyright 2019 AT&T Intellectual Property
 * Copyright 2019 Nokia
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>		//for close()
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>	//for inet_ntop()
#include <assert.h>
#include <netdb.h>

#include "e2sim_sctp.h"

const int CLIENT_SEND_COUNT         = 1;
const int SERVER_LISTEN_QUEUE_SIZE  = 10;

int sctp_start_server(const char *server_ip_str, const int server_port)
{
  if(server_port < 1 || server_port > 65535) {
      fprintf(stderr, "Invalid port number (%d). Valid values are between 1 and 65535.\n", server_port);
      return -1;
  }

  int server_fd;
  if((server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1) {
    perror("socket");
    return -1;
  }

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_family      = AF_INET;
  server_addr.sin_port        = htons(server_port);
  server_addr.sin_addr.s_addr = inet_addr(server_ip_str);

  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
    perror("bind");
    return -1;
  }

  if(listen(server_fd, SERVER_LISTEN_QUEUE_SIZE) != 0) {
    perror("listen");
    return -1;
  }

  assert(server_fd != 0);

  fprintf(stderr, "SCTP server started on %s:%d\n", server_ip_str, server_port);

  return server_fd;
}

int sctp_start_client(const char *server_name, const int server_port) {
  int client_fd;
  struct addrinfo hints, *server_info, *p;
  int rv;
  char port_str[6]; // Enough for 5 digits plus null terminator

  // Convert port to string for getaddrinfo
  snprintf(port_str, sizeof(port_str), "%d", server_port);

  // Initialize hints structure
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;     // IPv4
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_SCTP;

  // Get address information
  fprintf(stderr, "Resolving hostname %s...", server_name);
  if ((rv = getaddrinfo(server_name, port_str, &hints, &server_info)) != 0) {
      fprintf(stderr, "Failed: %s\n", gai_strerror(rv));
      return -1;
  }
  fprintf(stderr, "OK\n");

  // Loop through all the results and connect to the first one we can
  for (p = server_info; p != NULL; p = p->ai_next) {
      if ((client_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
          perror("socket");
          continue;
      }

      // Get the human-readable address for display
      char ip_str[INET_ADDRSTRLEN];
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, INET_ADDRSTRLEN);
      
      fprintf(stderr, "Connecting to server at %s:%d...", ip_str, server_port);
      if (connect(client_fd, p->ai_addr, p->ai_addrlen) == -1) {
          perror("connect");
          close(client_fd);
          continue;
      }

      // If we got here, we successfully connected
      break;
  }

  // Check if we connected successfully
  if (p == NULL) {
      fprintf(stderr, "Failed to connect to %s:%d\n", server_name, server_port);
      freeaddrinfo(server_info);
      return -1;
  }

  freeaddrinfo(server_info);

  assert(client_fd != 0);
  fprintf(stderr, "OK\n");
  return client_fd;
}

int sctp_start_client1(const char *server_ip_str, const int server_port)
{
  int client_fd;

  if((client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) == -1)
  {
     perror("socket");
     return -1;
  }

  struct sockaddr_in peer_addr;
  memset(&peer_addr, 0, sizeof(struct sockaddr_in));
  peer_addr.sin_family      = AF_INET;
  peer_addr.sin_port        = htons(server_port);
  peer_addr.sin_addr.s_addr = inet_addr(server_ip_str);
  // if(inet_pton(AF_INET, server_ip, &(peer_addr.sin_addr)) != 1) {
  //     printf("Error converting IP address (%s) to sockaddr_in structure\n", server_ip);
  //     return 0;
  // }

  fprintf(stderr, "Connecting to server at %s:%d ...", server_ip_str, server_port);
  if(connect(client_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) == -1) {
    perror("connect");
    return -1;
  }

  assert(client_fd != 0);

  fprintf(stderr, "OK\n");

  return client_fd;

}

//ssize_t sctp_send_to_socket(int sockfd, const void* buf, size_t len)
int sctp_send_to_socket(int sockfd, const void* buf, size_t len)
{
  int sent_len = 0;

  sent_len = send(sockfd, buf, len, 0);

  if(sent_len == -1)
  {
    perror("sctp_send_to_socket");
    return -1;
  }

  return sent_len;
}

// int sctp_recv_from_socket(int sockfd, void *buf, size_t buf_size)
// {
//   //int len = 0;
//
//   memset(buf, 0, buf_size);
//
//   int len = recv(sockfd, &buf, buf_size, 0);
//   if(len == -1)
//   {
//     perror("sctp_recv_from_socket");
//     return -1;
//   }
//
//   return len;
// }


//test only
void client_send_multiple_test_msg(int client_fd)
{
  char buf[1024];
  int SEND_COUNT = 4;

  for(int i = 0; i < SEND_COUNT; i++)
  {
    fprintf(stderr, "Sending message %d of %d \n", i+1, SEND_COUNT);

    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf)-1, "DATA %d", i+1);

    if(send(client_fd, &buf, strlen(buf), 0) == -1) {
      perror("send");
      return;
    }

    memset(buf, 0, sizeof(buf));

    if(recv(client_fd, &buf, sizeof(buf), 0) == -1) {
      perror("recv");
      return;
    }

    fprintf(stderr, "Server reply: %s\n", buf);

    sleep(1);
  }

  fprintf(stderr, "Closing...\n");
  if(close(client_fd) == -1) {
      perror("close");
      return;
  }
}
