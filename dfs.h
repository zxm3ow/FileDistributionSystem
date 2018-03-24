#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include <dictionary.h>

static typedef struct {
    int total;
    dictionary addr_map;
} dir_entry;

typedef struct { dir_entry entries[NUM_ENTRIES]; } dir;


typedef struct {
    uint32_t addr_start;
    size_t size;
} table_entry;

typedef struct { table_entry entries[NUM_ENTRIES]; } table;
