#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>

#include "api.h"
#include "map.h"
#include "sockheaders.h"
#include "socklib.h"


#define MAX_TOKENS  4
#define MAX_CLIENTS 128
#define BUFFER_SIZE 512


#define STDIN  0
#define STDOUT 1
#define STDERR 2


int eval_cmd(Command *cmd, Map *map) {

    /*
     * 0. l [list all]
     * 1. + k v [store key+value]
     * 2. ? k [get value by key]
     * 3. - k [remove value by key]
     * 4. # [count elements]
     *
     * typedef struct command {
     *     int8_t id;
     *     int8_t nfields;
     *     char fields[];
     * } Command;
     *
     */

    if (cmd->id == COM_LIST) {
        if (cmd->nfields != 0) {
            return ERRCODE;
        }
        for (size_t i = 0; i < map->count; ++i) {
            Entry *entry = &map->entries[i];
            printf("%s %s\n", entry->key, entry->value);
        }
        return NOERROR;
    }

    if (cmd->id == COM_PUT) {
        if (cmd->nfields != 2) {
            return ERRCODE;
        }

        char *idx = cmd->fields;
        Field *first = (Field *)idx;
        idx += ntohl(first->length) + sizeof(Field);
        Field *second = (Field *)idx;

        map_put(map, first->data, second->data);
        return NOERROR;
    }

    if (cmd->id == COM_GET) {
        if (cmd->nfields != 1) {
            return ERRCODE;
        }

        char *idx = cmd->fields;
        Field *field = (Field *)idx;

        char* value = map_get(map, field->data);
        if (value != NULL) {
            printf("%s\n", value);
        }
        return NOERROR;
    }

    if (cmd->id == COM_REMOVE) {
        if (cmd->nfields != 1) {
            return ERRCODE;
        }

        char *idx = cmd->fields;
        Field *field = (Field *)idx;

        map_remove(map, field->data);
        return NOERROR;
    }

    if (cmd->id == COM_COUNT) {
        if (cmd->nfields != 0) {
            return ERRCODE;
        }
        printf("%lu\n", map->count);
        return NOERROR;
    }

    return ERRCODE;
}

SOCKET handle_client(SOCKET client, char* buffer, Map *map) {
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

    // char **tokens;
    // size_t tokens_count = split_by_spaces(buffer, nrecvd, tokens, MAX_TOKENS);
    // Command cmd = cmd_init(tokens_count, tokens)
    // char *retval = eval_cmd(cmd);

    Command *cmd = (Command *)buffer;
    int ret = eval_cmd(cmd, map);

    // FIXME
    char retval[] = {ret}; // {0, 2, 0, 0, 0, 2, 'h', 'a', 0, 0, 0, 3, 'k', 'e', 'k'};
    tcp_send(client, retval, 15);

    fprintf(stdout, "%d: ", (int) client);
    fflush (stdout);
    write  (STDOUT, buffer, nrecvd);
    fprintf(stdout, "\n");
    return client;
}


int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s PORT\n", argv[0]);
        return 1;
    }

    char *host = "0.0.0.0";
    int   port = atoi(argv[1]);

    socklib_startup();

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

    Map *map = map_init(1);

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

            if (INVALID_SOCKET != handle_client(client, buffer, map)) {
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
    socklib_cleanup();
    return 0;
}
