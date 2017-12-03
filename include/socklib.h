#ifndef SOCKLIB_H_
#define SOCKLIB_H_

#include <stdbool.h>
#include <stdint.h>

#include "sockheaders.h"


int startup();

void cleanup();

int get_last_error();

SOCKET tcp_socket();

void set_address(struct sockaddr_in *sa, const char *host, uint16_t port);

int tcp_connect(SOCKET sock, const struct sockaddr_in *from_addr);

int tcp_bind(SOCKET sock, const struct sockaddr_in *from_addr);

int tcp_listen(SOCKET sock, int back_log);

SOCKET tcp_accept(SOCKET sock, const struct sockaddr_in *from_addr);

int tcp_send(SOCKET sock, const void *data, int len);

int tcp_recv(SOCKET sock, void *out_data, int recv_len);

int tcp_set_non_blocking(SOCKET sock, bool nya);

#endif  // SOCKLIB_H_
