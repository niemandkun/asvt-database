#ifndef CLIENT_H_
#define CLIENT_H_

#include "socklib.h"


typedef struct _client {
    SOCKET socket;
    int    env_size;
    char  *env;
}
client_t;


client_t c_create(SOCKET socket, size_t size);

void c_free(client_t *client);


#endif  // CLIENT_H
