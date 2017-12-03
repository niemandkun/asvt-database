#include <stdlib.h>
#include <string.h>

#if _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include "api.h"

Command *cmd_init(int argc, char **argv) {
    if (argc < 1) {
        return NULL;
    }

    if (strcmp(argv[0], "l") == 0) {
        if (argc != 1) {
            return NULL;
        }

        Command *cmd = malloc(sizeof(Command));
        if (cmd != NULL) {
            cmd->id = COM_LIST;
            cmd->nfields = 0;
        }
        return cmd;
    }

    if (strcmp(argv[0], "+") == 0) {
        if (argc != 3) {
            return NULL;
        }

        char *key = argv[1];
        char *value = argv[2];

        int32_t key_size = strlen(key);
        int32_t value_size = strlen(value);

        Command *cmd = malloc(
            sizeof(Command) + 2 * sizeof(Field) + key_size + value_size);

        if (cmd == NULL) {
            return NULL;
        }

        cmd->id = COM_PUT;
        cmd->nfields = 2;

        char* data = cmd->fields;

        Field *key_field = (Field *)data;
        Field *value_field = (Field *)(data + sizeof(Field) + key_size);

        key_field->length = htonl(key_size);
        memcpy(&key_field->data, key, key_size);

        value_field->length = htonl(value_size);
        memcpy(&value_field->data, value, value_size);

        return cmd;
    }

    if (strcmp(argv[0], "?") == 0) {
        if (argc != 2) {
            return NULL;
        }
        char *key = argv[1];
        int32_t key_size = strlen(key);

        Command *cmd = malloc(sizeof(Command) + sizeof(Field) + key_size);

        if (cmd == NULL) {
            return NULL;
        }

        Field *field = (Field *)cmd->fields;

        cmd->id = COM_GET;
        cmd->nfields = 1;

        field->length = htonl(key_size);
        memcpy(&field->data, key, key_size);

        return cmd;
    }

    if (strcmp(argv[0], "-") == 0) {
        if (argc != 2) {
            return NULL;
        }
        char *key = argv[1];
        int32_t key_size = strlen(key);

        Command *cmd = malloc(sizeof(Command) + sizeof(Field) + key_size);

        if (cmd == NULL) {
            return NULL;
        }

        Field *field = (Field *)cmd->fields;

        cmd->id = COM_REMOVE;
        cmd->nfields = 1;

        field->length = htonl(key_size);
        memcpy(&field->data, key, key_size);

        return cmd;
    }

    if (strcmp(argv[0], "#") == 0) {
        if (argc != 1) {
            return NULL;
        }

        Command *cmd = malloc(sizeof(Command));
        cmd->id = COM_COUNT;
        cmd->nfields = 0;

        return cmd;
    }

    return NULL;
}

size_t cmd_size(Command *cmd) {
    char nfields = cmd->nfields;
    char *data = cmd->fields;
    size_t cmd_size = sizeof(Command);

    for (int8_t i = 0; i < nfields; ++i) {
        Field *field = (Field *)data;
        size_t field_size = sizeof(Field) + ntohl(field->length);
        cmd_size += field_size;
        data += field_size;
    }

    return cmd_size;
}
