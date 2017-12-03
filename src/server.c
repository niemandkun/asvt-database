#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "sockheaders.h"
#include "socklib.h"


#define MAX_CLIENTS 128
#define BUFFER_SIZE 512


#define STDIN  0
#define STDOUT 1
#define STDERR 2

SOCKET handle_client(SOCKET client, char* buffer) {
    int nrecvd = tcp_recv(client, buffer, BUFFER_SIZE);
    if (nrecvd <= 0) {

        if (-nrecvd == WSAEWOULDBLOCK) {
            // data unavailable
            return client;
        }

        fprintf(stderr, "Client %d disconnected\n", (int) client);
        close(client);
        return INVALID_SOCKET;
    }

    fprintf(stdout, "%d: ", (int) client);
    fflush (stdout);
    write  (STDOUT, buffer, nrecvd);
    fprintf(stdout, "\n");
    return client;
}


int main(int argc, char *argv[]) { 

    if (argc != 3) {
        printf("Usage: %s host port\n", argv[0]);
        return 1;
    }

    char *host = argv[1];
    int   port = atoi(argv[2]);

    startup();

    SOCKET sock = tcp_socket();
    struct sockaddr_in addr;
    set_address(&addr, host, port);
    tcp_set_non_blocking(sock, true);

    int err;

    if ((err = tcp_bind(sock, &addr)) != NO_ERROR) return err;
    if ((err = tcp_listen(sock, 5))   != NO_ERROR) return err;

    SOCKET b_clients[MAX_CLIENTS] = { 0 };
    SOCKET b_clients_swp[MAX_CLIENTS] = { 0 };

    SOCKET *clients = b_clients;
    SOCKET *clients_swp = b_clients_swp;

    int    nclients = 0;

    char   buffer[512];

    for (;;) {

        if (nclients < MAX_CLIENTS) {
            SOCKET accepted = tcp_accept(sock, &addr);
            if (accepted != INVALID_SOCKET) {
                printf("Accepted %d\n", (int) accepted);
                tcp_set_non_blocking(accepted, true);
                clients[nclients++] = accepted;
            }
        }

        int nalive = 0;
        for (int i = 0; i < nclients; ++i) {
            SOCKET client = clients[i];
            if (INVALID_SOCKET == client)
                continue;

            if (INVALID_SOCKET != handle_client(client, buffer)) {
                clients_swp[nalive++] = client;
            }
        }
        nclients = nalive;

        SOCKET * t  = clients;
        clients     = clients_swp;
        clients_swp = t;
    }

    for (int i = 0; i < nclients; i++) {
        close(clients[i]);
    }
    close(sock);
    cleanup();
    return 0;
}