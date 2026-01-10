#include "ds/arena.h"
#include "utils.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN_UP_POW2(n, p)                                                    \
  ((uint64_t)n + ((uint64_t)p - 1)) & (~((uint64_t)p - 1))

#define ARENA_ALIGN (sizeof(void *))

void arena_init(mem_arena *arena, uint64_t capacity) {
  arena->buffer = (uint8_t *)scu_checked_malloc(capacity);
  arena->capacity = capacity;
  arena->pos = 0;
}

void arena_free(mem_arena *arena) {
  if (arena == NULL)
    return;
  free(arena->buffer);
  arena->buffer = NULL;
  arena->capacity = 0;
  arena->pos = 0;
}

void *arena_push(mem_arena *arena, uint64_t size) {
  uint64_t pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
  uint64_t new_pos = pos_aligned + size;

  if (new_pos > arena->capacity) {
    scu_perror(
        "Failed to allocate memory (requested more than available capacity)\n");

    return NULL;
  }

  void *out = arena->buffer + pos_aligned;
  arena->pos = new_pos;
  memset(out, 0, size);
  return out;
}

void arena_pop(mem_arena *arena, uint64_t size) {
  if (size > arena->pos)
    size = arena->pos;
  arena->pos -= size;
}

void arena_pop_to(mem_arena *arena, uint64_t pos) {
  if (pos <= arena->pos)
    arena->pos = pos;
}

void arena_clear(mem_arena *arena) { arena->pos = 0; }
