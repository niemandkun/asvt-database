#ifndef CLIENT_C_
#define CLIENT_C_

#include "client.h"
#include "socklib.h"


client_t c_create(SOCKET socket, size_t size) {
    client_t client;
    client.socket = socket;
    client.env = malloc(size);
    client.env_size = size;
    return client;
}

void c_free(client_t *client) {
    free(client->env);
    close(client->socket);
    client->socket = INVALID_SOCKET;
}


#endif