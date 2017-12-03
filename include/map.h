#ifndef _MAP_H_
#define _MAP_H_

#define ERRCODE -1
#define NOERROR 0

typedef struct entry {
    char *key;
    char *value;
} Entry;


typedef struct map {
    size_t size;
    size_t count;
    Entry *entries;
} Map;


/**
 * Returns new Map instance.
 * If malloc fails, returns NULL.
 */
Map *map_init(size_t initial_size);


/**
 * Cleanup. Should be called, if Map no more needed.
 */
void map_free(Map *map);


/**
 * Resize map. If a new size is less than the number of
 * the entries in the Map, redundant entries will be removed.
 *
 * If allocation fails, returns ERRCODE.
 */
int map_resize(Map *map, size_t size);


/**
 * Put value in a Map for a given key.
 *
 * If key is not present in the Map and the Map is
 * too small to contain one more entry, size of the Map
 * will be increased automatically.
 *
 * new_size = 2 * old_size
 *
 * If error occured, returns ERROR.
 */
int map_put(Map *map, char *key, char *value);


/**
 * Get last saved value from a Map for a given key.
 * If the given key is not present in the Map, returns NULL.
 */
char *map_get(Map *map, char *key);


void map_print(Map *map);

#endif
