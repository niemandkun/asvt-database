#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "sockheaders.h"
#include "socklib.h"
#include "client.h"


#define MAX_CLIENTS 128
#define BUFFER_SIZE 1024


#define STDIN  0
#define STDOUT 1
#define STDERR 2

void handle_client(client_t *client) {
    int nrecvd = tcp_recv(client->socket, client->env, client->env_size);
    if (nrecvd <= 0) {

        if (-nrecvd == WSAEWOULDBLOCK) {
            // data unavailable
            return;
        }

        fprintf(stderr, "Client %d disconnected\n", (int) client->socket);
        c_free(client);
        return;
    }

    fprintf(stdout, "%d: ", (int) client->socket);
    fflush (stdout);
    write  (STDOUT, client->env, nrecvd);
    fprintf(stdout, "\n");
    return;
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

    client_t b_clients[MAX_CLIENTS];
    client_t b_clients_swp[MAX_CLIENTS];

    client_t *clients = b_clients;
    client_t *clients_swp = b_clients_swp;

    int    nclients = 0;

    for (;;) {

        if (nclients < MAX_CLIENTS) {
            SOCKET accepted = tcp_accept(sock, &addr);
            if (accepted != INVALID_SOCKET) {
                printf("Accepted %d\n", (int) accepted);
                tcp_set_non_blocking(accepted, true);
                clients[nclients++] = c_create(accepted, BUFFER_SIZE);
            }
        }

        int nalive = 0;
        for (int i = 0; i < nclients; ++i) {
            client_t client = clients[i];
            if (INVALID_SOCKET == client.socket)
                continue;

            handle_client(&client);

            if (INVALID_SOCKET != client.socket) {
                clients_swp[nalive++] = client;
            }
        }
        nclients = nalive;

        client_t *t = clients;
        clients     = clients_swp;
        clients_swp = t;
    }

    for (int i = 0; i < nclients; i++) {
        c_free(&clients[i]);
    }
    close(sock);
    cleanup();
    return 0;
}
