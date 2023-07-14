/* Defines the hash map data structure for mapping state names to their signals, and signals to their values. */
#ifndef _HASHMAP_H_
#define _HASHMAP_H_
#include <stdint.h>

// Subject to change if more signals are added
#define signal_bf_t uint64_t
#define STATE_ADDRESS_BITS 6

/* Hash map entries (chained) */
typedef struct Entry {
    const char *name;
    signal_bf_t value;
    struct Entry *next;
} hmap_entry_t;

/* Hash map data structure */
typedef struct StateHashmap {
    unsigned __backing_len;
    unsigned key_count;
    hmap_entry_t **entries;
    unsigned (*hash)(const char *, unsigned);
} hmap_t;

hmap_t *hmap_construct(unsigned length);
void hmap_destruct(hmap_t *hmap);

void hmap_add_entry(hmap_t *hmap, const char *name, signal_bf_t value);
signal_bf_t *hmap_get(hmap_t *hmap, const char *name);

hmap_t *valid_signals(void);
#endif // _HASHMAP_H_
