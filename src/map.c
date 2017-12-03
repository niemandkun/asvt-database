#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "map.h"

Map *map_init(size_t size) {
    Map *map = malloc(sizeof(Map));

    if (map == NULL) {
        return map;
    }

    map->size = size;
    map->count = 0;
    map->entries = calloc(map->size, sizeof(Entry));

    return map;
}

void map_free(Map *map) {
    for (size_t i = 0; i < map->count; ++i) {
        Entry *entry = &map->entries[i];
        free(entry->key);
        free(entry->value);
    }
    free(map->entries);
    free(map);
}

int map_resize(Map *map, size_t size) {
    Entry *new_entries = calloc(size, sizeof(Entry));

    if (new_entries == NULL) {
        return ERRCODE;
    }

    map->count = map->count < size ? map->count : size;

    for (size_t i = 0; i < map->count; ++i) {
        new_entries[i] = map->entries[i];
    }

    map->entries = new_entries;
    map->size = size;

    return 0;
}

static int map_replace(Map *map, char *key, char *value) {
    for (size_t i = 0; i < map->count; ++i) {
        Entry *entry = &map->entries[i];

        if (entry-> key != NULL && (entry->key == key || strcmp(key, entry->key) == 0)) {
            entry->value = value;
            return 1;
        }
    }

    return 0;
}

static int map_put_new(Map *map, char *key, char *value) {
    if (map->count == map->size) {

        int errcode = map_resize(map, 2 * map->size);

        if (errcode != NOERROR) {
            return 0;
        }
    }

    Entry *entry = &map->entries[map->count++];

    entry->key = key;
    entry->value = value;

    return 1;
}

int map_put(Map *map, char *key, char *value) {
    if (key == NULL || value == NULL) {
        return ERRCODE;
    }

    if (map_replace(map, key, value)) {
        return NOERROR;
    }

    if (map_put_new(map, key, value)) {
        return NOERROR;
    }

    return ERRCODE;
}

char *map_get(Map *map, char *key) {
    for (size_t i = 0; i < map->count; ++i) {
        Entry *entry = &map->entries[i];

        if (entry->key != NULL && strcmp(key, entry->key) == 0) {
            return entry->value;
        }
    }

    return NULL;
}

void map_print(Map *map) {
    printf("Map {\n    size=%lu,\n    count=%lu,\n    entries=[\n", map->size, map->count);

    for (size_t i = 0; i < map->count; ++i) {
        Entry *entry = &map->entries[i];
        printf("        Entry { key=\"%s\", value=\"%s\" },\n", entry->key, entry->value);
    }

    printf("    ],\n}\n");
}
