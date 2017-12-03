#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

#define MAX_INPUT 1024
#define MAX_TOKENS 3

size_t getinput(char *buffer, int max_len) {
    fgets(buffer, max_len, stdin);
    int len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        return len - 1;
    }

    return len;
}

size_t split_by_spaces(char* source, size_t source_size,
                       char** dest, size_t dest_size) {

    if (dest_size == 0) {
        return 0;
    }

    for (size_t i = 0; i < source_size; ++i) {
        if (source[i] == ' ' || source[i] == '\t' || source[i] == '\n') {
            source[i] = '\0';
        }
    }

    for (size_t token = 0, i = 0; token < dest_size; ++token) {
        while (source[i] == '\0' && i < source_size) {
            i++;
        }

        if (i >= source_size) {
            return token;
        }

        char *cpy = malloc(source_size);

        if (cpy == NULL) {
            return token;
        }

        strcpy(cpy, &source[i]);
        int cpy_size = strlen(cpy);
        dest[token] = realloc(cpy, cpy_size + 1);

        if (cpy == NULL) {
            return token;
        }

        i += cpy_size;
    }

    return dest_size;
}

int perform_command(Map *map, size_t argc, char** argv) {
    if (argc < 1) {
        return ERRCODE;
    }

    if (strcmp(argv[0], "??") == 0) {
        if (argc != 1) {
            return ERRCODE;
        }
        printf("Help!\n");
        return NOERROR;
    }

    if (strcmp(argv[0], "l") == 0) {
        if (argc != 1) {
            return ERRCODE;
        }
        for (size_t i = 0; i < map->count; ++i) {
            Entry *entry = &map->entries[i];
            printf("%s %s\n", entry->key, entry->value);
        }
        return NOERROR;
    }

    if (strcmp(argv[0], "p") == 0) {
        if (argc != 1) {
            return ERRCODE;
        }
        map_print(map);
        return NOERROR;
    }

    if (strcmp(argv[0], "+") == 0) {
        if (argc != 3) {
            return ERRCODE;
        }
        map_put(map, argv[1], argv[2]);
        return NOERROR;
    }

    if (strcmp(argv[0], "?") == 0) {
        if (argc != 2) {
            return ERRCODE;
        }
        char* value = map_get(map, argv[1]);
        if (value != NULL) {
            printf("%s\n", value);
        }
        return NOERROR;
    }

    if (strcmp(argv[0], "-") == 0) {
        if (argc != 2) {
            return ERRCODE;
        }
        map_remove(map, argv[1]);
        return NOERROR;
    }

    if (strcmp(argv[0], "#") == 0) {
        if (argc != 1) {
            return ERRCODE;
        }
        printf("%lu\n", map->count);
        return NOERROR;
    }

    return ERRCODE;
}

int main(void) {
    Map *map = map_init(1);

    char **tokens = calloc(MAX_TOKENS, sizeof(char *));
    char *input = malloc(MAX_INPUT);

    while (1) {
        size_t input_size = getinput(input, MAX_INPUT);

        if (input_size == 0) {
            break;
        }

        size_t tokens_count = split_by_spaces(input, input_size, tokens, MAX_TOKENS);

        if (perform_command(map, tokens_count, tokens) != NOERROR) {
            printf("Error!\n");
        }
    }

    map_free(map);
    return 0;
}
