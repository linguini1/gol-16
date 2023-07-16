/* Implements the hash map data structure for mapping state names to their signals, and signals to their values. */
#include "hashmap.h"
#include <math.h>
#include <stdio.h> // TODO remove
#include <stdlib.h>
#include <string.h>

static const char *VALID_SIGNALS[] = {
    "t1oe", "t1ce", "t2oe",  "t2ce",  "rd",    "rx",    "ry",     "regw",   "regr",   "pcoe",    "spoe", "lroe",
    "froe", "frce", "ui4",   "ui7",   "ui9",   "si7",   "si9",    "aadd",   "asub",   "anop",    "aop",  "coe",
    "irce", "iroe", "marce", "maroe", "mdrce", "mdroe", "mdrput", "mdrget", "ibread", "ibwrite",
};
static const unsigned SIGNAL_COUNT = sizeof(VALID_SIGNALS) / sizeof(char *);

/* Internal hash function */
static unsigned hash_fn(const char *name, unsigned table_len) {
    unsigned value = 0;
    while (*name++)
        value += *name;
    return value % table_len;
}

/* Creates a new hash map entry. */
static hmap_entry_t *entry_construct(const char *name, signal_bf_t value, hmap_entry_t *next) {
    hmap_entry_t *entry = malloc(sizeof(hmap_entry_t));
    entry->name = name;
    entry->value = value;
    entry->next = next;
    return entry;
}

/* Destroys hash map entry chain in sequence. */
static void entry_destruct(hmap_entry_t *entry) {
    if (entry == NULL) return;
    for (hmap_entry_t *cur = entry; cur->next != NULL;) {
        hmap_entry_t *next = cur->next;
        free(cur);
        cur = next;
    }
}

/* Creates a new hash map */
hmap_t *hmap_construct(unsigned length) {
    hmap_t *map = malloc(sizeof(hmap_t));
    map->__backing_len = length;
    map->key_count = 0;
    map->entries = malloc(sizeof(hmap_entry_t) * length);
    for (unsigned i = 0; i < length; i++) {
        map->entries[i] = NULL; // Mark as empty
    }
    map->hash = hash_fn;
    return map;
}

/* Destroys a hash map */
void hmap_destruct(hmap_t *hmap) {
    for (unsigned i = 0; i < hmap->__backing_len; i++)
        entry_destruct(hmap->entries[i]);
    free(hmap);
}

/* Adds an entry to the hash map */
void hmap_add_entry(hmap_t *hmap, const char *name, signal_bf_t value) {
    unsigned index = hmap->hash(name, hmap->__backing_len);
    hmap->key_count++;
    hmap_entry_t *entry = hmap->entries[index];

    // Assigned index is empty
    if (entry == NULL) {
        hmap->entries[index] = entry_construct(name, value, NULL);
        return;
    }

    // Check if name already exists
    for (; strcmp(entry->name, name) && entry->next != NULL; entry = entry->next)
        ;

    if (entry->next == NULL) {
        // Must add to chain
        hmap_entry_t *new = entry_construct(name, value, hmap->entries[index]);
        hmap->entries[index] = new;
        return;
    }

    // Name exists, update value
    entry->value = value;
}

/* Gets an entry from the hash map */
signal_bf_t *hmap_get(hmap_t *hmap, const char *name) {
    hmap_entry_t *entry = hmap->entries[hmap->hash(name, hmap->__backing_len)];
    if (entry == NULL) return NULL;

    // Cycle through chain until key matches
    for (; strcmp(entry->name, name) && entry->next != NULL; entry = entry->next)
        ;

    if (!strcmp(entry->name, name)) return &entry->value;
    return NULL;
}

/* Initializes a hash map of valid signals. */
hmap_t *valid_signals(void) {
    hmap_t *signals = hmap_construct(SIGNAL_COUNT / 2);
    for (unsigned i = 0; i < SIGNAL_COUNT; i++) {
        signal_bf_t field = powl(2, i);
        field = field << STATE_ADDRESS_BITS; // LSBs reserved for next state address
        hmap_add_entry(signals, VALID_SIGNALS[i], field);
    }
    return signals;
}
