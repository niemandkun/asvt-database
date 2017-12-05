#ifndef _API_H_
#define _API_H_

#include<stdint.h>

#define COM_LIST 0
#define COM_PUT 1
#define COM_GET 2
#define COM_REMOVE 3
#define COM_COUNT 4

typedef struct field {
    int32_t length; // Network byte order! Use ntohl to read this field.
    char data[];
} Field;

typedef struct command {
    int8_t id;
    int8_t nfields;
    char fields[];
} Command;

/**
 * Returns Command or NULL if initialization fails.
 */
Command *cmd_init(int argc, char **argv);

/**
 * Returns full size of a given Command in bytes.
 */
size_t cmd_size(Command *cmd);

#endif
