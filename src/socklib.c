#ifndef _SOCKLIB_C_
#define _SOCKLIB_C_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "sockheaders.h"
#include "socklib.h"


int startup() {
  #ifdef _WIN32
    WSADATA WSAData;
    int errcode = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (errcode != 0) {
        fprintf(stderr, "%s: %d\n", "WSA Startup error", errcode);
        return -1;
    }
  #endif
    return 0;
}

void cleanup() {
  #ifdef _WIN32
    WSACleanup();
  #endif
}

int get_last_error() {
  #ifdef _WIN32
    return WSAGetLastError();
  #else
    return errno;
  #endif
}

SOCKET tcp_socket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "%s\n", "tcp socket craetion error");
    }
    return sock;
}

void set_address(struct sockaddr_in *sa, const char *host, uint16_t port) {
    sa->sin_family = AF_INET;
    inet_pton(AF_INET, host, &sa->sin_addr);
    sa->sin_port = htons(port);
}

int tcp_connect(SOCKET sock, const struct sockaddr_in *from_addr) {
    int err = connect(sock, (struct sockaddr *) from_addr, sizeof(struct sockaddr));
    if (err != 0) {
        fprintf(stderr, "%s: %d\n", "tcp socket conntect error", err);
        return -get_last_error();
    }
    return NO_ERROR;
}

int tcp_bind(SOCKET sock, const struct sockaddr_in *from_addr) {
    int err = bind(sock, (struct sockaddr *) from_addr, sizeof(struct sockaddr));
    if (err != 0) {
        fprintf(stderr, "%s: %d\n", "tcp socket bind error", err);
        return -get_last_error();
    }
    return NO_ERROR;
}

int tcp_listen(SOCKET sock, int back_log) {
    int err = listen(sock, back_log);
    if (err < 0) {
        fprintf(stderr, "%s: %d\n", "tcp socket listen error", err);
        return -get_last_error();
    }
    return NO_ERROR;
}


SOCKET tcp_accept(SOCKET sock, const struct sockaddr_in *from_addr) {
    socklen_t socklen = (int) sizeof(struct sockaddr);
    return accept(sock, (struct sockaddr *) from_addr, &socklen);
}


int tcp_send(SOCKET sock, const void *data, int len) {
    int nsent = send(sock, (const char *) data, len, 0);
    if (nsent < 0) {
        int err = -get_last_error();
        fprintf(stderr, "%s: %d\n", "tcp send error", err);
        return err;
    }
    return nsent;
}

int tcp_recv(SOCKET sock, void *out_data, int recv_len) {
    int nrecv = recv(sock, (char *) out_data, recv_len, 0);
    if (nrecv < 0) {
        int err = -get_last_error();

        if (WSAEWOULDBLOCK == -err) {
            return err;
        }

        fprintf(stderr, "%s: %d\n", "tcp recv error", err);
        return err;
    }
    return nrecv;
}

int tcp_set_non_blocking(SOCKET sock, bool nya) {
    int result = NO_ERROR;
  #if _WIN32
    u_long arg = nya ? 1 : 0;
    result = ioctlsocket(sock, FIONBIO, &arg);
  #else
    int flags = fcntl(sock, F_GETFL, 0);
    flags = nya ? (flags | O_NONBLOCK):(flags & ~O_NONBLOCK);
    fcntl(sock, F_SETFL, flags);
  #endif
    if (result == SOCKET_ERROR) {
        fprintf(stderr, "%s: %d\n", "tcp set non blocking error", result);
        return -get_last_error();
    }
    return NO_ERROR;
}

#endif