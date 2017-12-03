#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "api.h"
#include "socklib.h"
#include "utils.h"

#define STDOUT 1

#define INPUT_BUFFER_SIZE 1024
#define FILE_BUFFER_SIZE 1024
#define NETWORK_BUFFER_SIZE 1024
#define MAX_TOKENS 4

#define ERRCODE -1
#define NOERROR 0

char **tokens;
char *input_buffer;
char *network_buffer;

SOCKET sock;

size_t getinput() {
    fgets(input_buffer, INPUT_BUFFER_SIZE, stdin);
    int len = strlen(input_buffer);

    if (len > 0 && input_buffer[len - 1] == '\n') {
        input_buffer[len - 1] = '\0';
        return len - 1;
    }

    return len;
}

Command *convert_to_command(size_t argc, char** argv) {
    Command *command = cmd_init(argc, argv);

    if (command == NULL) {
        return NULL;
    }

    char *payload = (char *)command;

    for (size_t i = 0, n = cmd_size(command); i < n; ++i) {
        printf("%02X ", payload[i]);
    }

    printf("\n");

    return command;
}

int perform_remotely(size_t argc, char **argv) {
    Command *command = convert_to_command(argc, argv);

    if (command == NULL) {
        return ERRCODE;
    }

    int to_send = cmd_size(command);
    int nsent = tcp_send(sock, command, to_send);

    if (nsent != to_send) {
        return ERRCODE;
    }

    tcp_recv(sock, network_buffer, NETWORK_BUFFER_SIZE);

    Command *from_server = (Command *)network_buffer;
    char *idx = from_server->fields;

    for (int i = 0; i < from_server->nfields; ++i) {
        Field *field = (Field *) idx;
        int field_len = ntohl(field->length);
        write(STDOUT, field->data, field_len);
        printf("\n");
        idx += field_len + sizeof(Field);
    }

    return NOERROR;
}

int perform_locally(size_t argc, char** argv) {
    if (argc < 1) {
        return ERRCODE;
    }

    char *command = argv[0];

    if (strcmp(command, "??") == 0) {
        if (argc != 1) {
            return ERRCODE;
        }

        printf("Available commands:\n");
        printf("??              show this help message\n");
        printf("l               list all keys\n");
        printf("+ KEY VALUE     store key and value on server\n");
        printf("+ FILE          store content of file on server\n");
        printf("? KEY           lookup key\n");
        printf("- KEY           remove key\n");
        printf("#               print number of keys on server\n");

        return NOERROR;
    }

    if (strcmp(command, "+") == 0 && argc == 2) {
        char *filename = argv[1];

        int file = open(filename, O_RDWR);
        if (file == -1) {
            return ERRCODE;
        }

        char *content = malloc(FILE_BUFFER_SIZE);
        if (content == NULL) {
            return ERRCODE;
        }

        read(file, content, FILE_BUFFER_SIZE);
        close(file);

        char *fake_argv[3];
        fake_argv[0] = command;
        fake_argv[1] = filename;
        fake_argv[2] = content;

        perform_remotely(3, fake_argv);
        free(content);
        return NOERROR;
    }

    return ERRCODE;
}

SOCKET init_socket(char* host, uint16_t port) {
    socklib_startup();
    SOCKET sock = tcp_socket();
    struct sockaddr_in addr;
    set_address(&addr, host, port);
    tcp_connect(sock, &addr);
    return sock;
}

void init(char **argv) {
    sock = init_socket(argv[1], atoi(argv[2]));
    tokens = calloc(MAX_TOKENS, sizeof(char *));
    input_buffer = malloc(INPUT_BUFFER_SIZE);
    network_buffer = malloc(NETWORK_BUFFER_SIZE);
}

void cleanup() {
    close(sock);
    socklib_cleanup();
    free(tokens);
    free(input_buffer);
    free(network_buffer);
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s HOST PORT\n", argv[0]);
        return 1;
    }

    init(argv);

    while (1) {
        size_t input_size = getinput(input_buffer, INPUT_BUFFER_SIZE);

        if (input_size == 0) {
            continue;
        }

        size_t ntokens =
            split_by_spaces(input_buffer, input_size, tokens, MAX_TOKENS);

        if (perform_locally(ntokens, tokens) == NOERROR) {
            continue;
        }

        if (perform_remotely(ntokens, tokens) == NOERROR) {
            continue;
        }

        printf("error\n");
    }

    cleanup();
    return 0;
}
