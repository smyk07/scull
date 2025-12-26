#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

typedef struct mem_arena mem_arena;

mem_arena *arena_create(uint64_t capacity);
void arena_destroy(mem_arena *arena);

void *arena_push(mem_arena *arena, uint64_t size);
void arena_pop(mem_arena *arena, uint64_t size);
void arena_pop_to(mem_arena *arena, uint64_t pos);
void arena_clear(mem_arena *arena);

#define arena_push_struct(arena, T) (T *)arena_push((arena), sizeof(T))

#endif // !ARENA_H
