#include <stdio.h>
#include <string.h>
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


Command *eval_cmd(Command *cmd, Map *map) {
    char err_cmd_arr[] = {0, 1, 0, 0, 0, 5, 'e', 'r', 'r', 'o', 'r'};
    Command *err_allocated = malloc(11);
    memcpy(err_allocated, err_cmd_arr, 11);

    char ok_cmd_arr[] = {0, 1, 0, 0, 0, 2, 'o', 'k'};
    Command *ok_allocated = malloc(8);
    memcpy(ok_allocated, ok_cmd_arr, 8);

    Field *field;
    Entry *entry;
    Command *result;

    /*
     * 0. l [list all]
     * 1. + k v [store key+value]
     * 2. ? k [get value by key]
     * 3. - k [remove value by key]
     * 4. # [count elements]
     */

    if (cmd->id == COM_LIST) {
        if (cmd->nfields != 0) {
            free(ok_allocated);
            return err_allocated;
        }
        int counter = sizeof(Command);
        for (size_t i = 0; i < map->count; ++i) {
            entry = &map->entries[i];
            counter += strlen(entry->key);
            counter += strlen(entry->value);
            counter += sizeof(Field) + 1;
        }
        result = malloc(counter);
        result->id = 0;
        result->nfields = (int8_t)map->count;
        char *idx = result->fields;

        for (size_t i = 0; i < map->count; ++i) {
        // 1: 0 | 1: fields num | 4: first len | N: first bytes | ... |
            entry = &map->entries[i];

            field = (Field *)idx;

            int key_len = strlen(entry->key);
            int value_len = strlen(entry->value);
            int field_len = key_len + value_len + 1;
            field->length = htonl(field_len);

            printf("field len: %d\n", field_len);

            idx = (char *)field->data;
            memcpy(idx, entry->key, key_len);

            idx += key_len;
            idx[0] = ' ';

            idx += 1;
            memcpy(idx, entry->value, value_len);

            idx += value_len;
        }

        free(err_allocated);
        free(ok_allocated);
        return result;
    }

    if (cmd->id == COM_PUT) {
        if (cmd->nfields != 2) {
            free(ok_allocated);
            return err_allocated;
        }

        char *idx = cmd->fields;
        Field *first = (Field *)idx;
        int first_length = ntohl(first->length);
        idx += first_length + sizeof(Field);
        Field *second = (Field *)idx;
        int second_length = ntohl(second->length);

        // printf("first: '%s', second: '%s'\n", first->data, second->data);

        char *d1 = malloc(first_length + 1);
        char *d2 = malloc(second_length + 1);

        memcpy(d1, first->data, first_length);
        d1[first_length] = 0;

        memcpy(d2, second->data, second_length);
        d2[second_length] = 0;

        if (!map_put(map, d1, d2)) {
            free(err_allocated);
            return ok_allocated;
        } else {
            free(ok_allocated);
            return err_allocated;
        }
    }

    if (cmd->id == COM_GET) {
        if (cmd->nfields != 1) {
            free(ok_allocated);
            return err_allocated;
            // return ERRCODE;
        }

        char *idx = cmd->fields;
        field = (Field *)idx;
        int field_len = ntohl(field->length);

        char *key = malloc(field_len + 1);

        memcpy(key, field->data, field_len);
        key[field_len] = 0;

        char* value = map_get(map, key);

        if (value == NULL) {
            free(ok_allocated);
            return err_allocated;
        }


        int value_len = strlen(value);

        int command_len = sizeof(Command) + sizeof(Field) + value_len;

        result = malloc(command_len);
        result->id = 0;
        result->nfields = 1;

        field = (Field *)result->fields;
        field->length = htonl(value_len);

        memcpy(field->data, value, value_len);

        free(err_allocated);
        free(ok_allocated);

        return result;
    }

    if (cmd->id == COM_REMOVE) {
        if (cmd->nfields != 1) {
            free(ok_allocated);
            return err_allocated;
            // return ERRCODE;
        }

        char *idx = cmd->fields;
        field = (Field *)idx;
        int field_len = ntohl(field->length);

        char *key = malloc(field_len + 1);

        memcpy(key, field->data, field_len);
        key[field_len] = 0;

        map_remove(map, key);

        free(err_allocated);
        return ok_allocated;
    }

    if (cmd->id == COM_COUNT) {
        if (cmd->nfields != 0) {
            free(ok_allocated);
            return err_allocated;
            // return ERRCODE;
        }

        char *resp = calloc(1024, sizeof(char));

        sprintf(resp, "%lu\n", map->count);
        int resp_len = strlen(resp);

        result = malloc(sizeof(Command) + sizeof(Field) + resp_len);
        result->id = 0;
        result->nfields = 1;

        field = (Field *)result->fields;
        field->length = htonl(resp_len);

        memcpy(field->data, resp, resp_len);

        free(err_allocated);
        free(ok_allocated);

        return result;
    }

    free(ok_allocated);
    return err_allocated;
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

    Command *cmd = (Command *)buffer;

    printf("Command comes to server:\n");
    char *payload = (char *)cmd;
    for (size_t i = 0, n = cmd_size(cmd); i < n; ++i) {
        printf("%02X ", payload[i]);
    }
    printf("\n");

    Command *ret = eval_cmd(cmd, map);

    printf("Command comes to client:\n");
    payload = (char *)ret;
    for (size_t i = 0, n = cmd_size(ret); i < n; ++i) {
        printf("%02X ", payload[i]);
    }
    printf("\n");

    tcp_send(client, ret, cmd_size(ret));

    free(ret);

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

    char   buffer[512000];

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
