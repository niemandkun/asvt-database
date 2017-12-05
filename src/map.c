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

    free(map->entries);

    map->entries = new_entries;
    map->size = size;

    return 0;
}

static int has_key(Entry *entry, char *key) {
    printf("has_key(): entry->key: '%s' key: '%s'\n", entry->key, key);
    return entry-> key != NULL
        && (entry->key == key || strcmp(key, entry->key) == 0);
}

static int find_entry_by_key(Entry *entries, size_t count, char *key) {
    for (size_t i = 0; i < count; ++i) {
        if (has_key(&entries[i], key)) {
            return i;
        }
    }
    return ERRCODE;
}

static int map_replace(Map *map, char *key, char *value) {
    int i = find_entry_by_key(map->entries, map->count, key);
    printf("map_replace(): found %s at %d\n", key, i);
    if (i >= 0) {
        Entry *entry = &map->entries[i];
        printf("map_replace(): entry addr == %p\n", (void *)entry);
        entry->value = value;
        return NOERROR;
    }
    return ERRCODE;
}

static int map_put_new(Map *map, char *key, char *value) {
    if (map->count == map->size) {
        int errcode = map_resize(map, 2 * map->size);
        if (errcode != NOERROR) {
            return errcode;
        }
    }

    Entry *entry = &map->entries[map->count++];
    printf("put_new(): entry == %p \n", (void *)entry);

    entry->key = key;
    entry->value = value;

    return NOERROR;
}

int map_put(Map *map, char *key, char *value) {
    if (key == NULL || value == NULL) {
        return ERRCODE;
    }

    if (!map_replace(map, key, value)) {
        return NOERROR;
    }

    if (!map_put_new(map, key, value)) {
        return NOERROR;
    }

    return ERRCODE;
}

char *map_get(Map *map, char *key) {
    int i = find_entry_by_key(map->entries, map->count, key);
    if (i >= 0) {
        Entry *entry = &map->entries[i];
        return entry->value;
    }
    return NULL;
}

void map_remove(Map *map, char *key) {
    int i = find_entry_by_key(map->entries, map->count, key);

    if (i < 0) {
        return;
    }

    Entry *entry = &map->entries[i];

    printf("key: '%s', value: '%s'\n", entry->key, entry->value);

    free(entry->key);
    free(entry->value);

    size_t j = i;

    while (j < map->count - 1) {
        map->entries[j] = map->entries[j + 1];
        j++;
    }

    map->entries[j].key = NULL;
    map->entries[j].value = NULL;

    map->count--;
}

void map_print(Map *map) {
    long unsigned int map_size = (long unsigned int) map->size;
    long unsigned int map_count = (long unsigned int) map->count;
    printf("Map {\n    size=%lu,\n    count=%lu,\n    entries=[\n", map_size, map_count);

    for (size_t i = 0; i < map->count; ++i) {
        Entry *entry = &map->entries[i];
        printf("        Entry { key=\"%s\", value=\"%s\" },\n", entry->key, entry->value);
    }

    printf("    ],\n}\n");
}

// char *map_list(Map *map) {
//     long unsigned int map_size = (long unsigned int) map->size;
//     long unsigned int map_count = (long unsigned int) map->count;
//     char *result = "";
//
//     for (size_t i = 0; i < map->count; ++i) {
//         Entry *entry = &map->entries[i];
//         result = strcat(result, entry->key);
//         result = strcat(result, " ");
//         result = strcat(result, entry->value);
//         result = strcat(result, "\n");
//     }
//
//     return result;
// }
