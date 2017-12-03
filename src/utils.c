#include <stdlib.h>
#include <string.h>
#include "utils.h"

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

