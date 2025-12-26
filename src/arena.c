#include "arena.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

#define ALIGN_UP_POW2(n, p)                                                    \
  ((uint64_t)n + ((uint64_t)p - 1)) & (~((uint64_t)p - 1))

struct mem_arena {
  uint64_t capacity;
  uint64_t pos;
};

#define ARENA_BASE_POS (sizeof(mem_arena))
#define ARENA_ALIGN (sizeof(void *))

mem_arena *arena_create(uint64_t capacity) {
  mem_arena *arena = (mem_arena *)malloc(ARENA_BASE_POS + capacity);
  arena->capacity = capacity;
  arena->pos = ARENA_BASE_POS;
  return arena;
}

void arena_destroy(mem_arena *arena) { free(arena); }

void *arena_push(mem_arena *arena, uint64_t size) {
  uint64_t pos_aligned = ALIGN_UP_POW2(arena->pos, ARENA_ALIGN);
  uint64_t new_pos = pos_aligned + size;

  if (new_pos > ARENA_BASE_POS + arena->capacity) {
    scu_perror(
        NULL,
        "Failed to allocate memory (requested more than available capacity)\n");
    arena_destroy(arena);
    return NULL;
  }

  arena->pos = new_pos;
  uint8_t *out = (uint8_t *)arena + pos_aligned;
  memset(out, 0, size);

  return out;
}

void arena_pop(mem_arena *arena, uint64_t size) {
  size =
      ((size) < (arena->pos - ARENA_BASE_POS) ? (size)
                                              : (arena->pos - ARENA_BASE_POS));

  arena->pos -= size;
}

void arena_pop_to(mem_arena *arena, uint64_t pos) {
  uint64_t size = (pos < arena->pos) ? (arena->pos - pos) : 0;
  arena_pop(arena, size);
}

void arena_clear(mem_arena *arena) { arena_pop_to(arena, ARENA_BASE_POS); }
