#ifndef DYNAMIC_ARRAY
#define DYNAMIC_ARRAY

#include "common.h"

typedef struct dynamic_array {
  void *items;
  u64 item_size;
  u64 count;
  u64 capacity;
} dynamic_array;

void dynamic_array_init(dynamic_array *da, u64 size);

u32 dynamic_array_get(dynamic_array *da, u64 index, void *item);

u32 dynamic_array_set(dynamic_array *da, u64 index, void *item);

u32 dynamic_array_append(dynamic_array *da, void *item);

u32 dynamic_array_insert(dynamic_array *da, u64 index, void *item);

u32 dynamic_array_remove(dynamic_array *da, u64 index);

u32 dynamic_array_pop(dynamic_array *da, void *item);

void dynamic_array_free(dynamic_array *da);

void dynamic_array_free_items(dynamic_array *da);

#endif // !DYNAMIC_ARRAY
