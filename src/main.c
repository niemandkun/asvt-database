#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"

#define MAX_INPUT_LEN 1024

int getinput(char *buffer, int max_len) {
    fgets(buffer, max_len, stdin);
    int len = strlen(buffer);

    if (len == 0) {
        return 0;
    }

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        return len - 1;
    }

    return len;
}

int main(void) {
    Map *map = map_init(1);

    char *input = malloc(MAX_INPUT_LEN);
    int input_len = 0;

    while (1) {
        input_len = getinput(input, MAX_INPUT_LEN);

//#ifdef _DEBUG_
        printf("ECHO (%d): %s\n", input_len, input);
//#endif

        char *space_ptr = strchr(input, ' ');

        if (space_ptr == NULL) {

            printf("%s\n", map_get(map, input));

        } else {

            *space_ptr = 0;
            map_put(map, input, space_ptr + 1);

        }
    }

    map_free(map);

    return 0;
}
