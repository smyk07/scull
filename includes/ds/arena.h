/*
 * arena: memory arena allocator for fast bulk allocation.
 *
 * initial implementation inspired from "i hate malloc/free with a passion" by
 * MagicalBat on YouTube (https://youtu.be/jgiMagdjA1s)
 */

#ifndef ARENA_H
#define ARENA_H

#include <stdint.h>

typedef struct mem_arena {
  uint64_t capacity;
  uint64_t pos;
  uint8_t *buffer;
} mem_arena;

/*
 * @brief: Initializes an already allocated memory arena with the specified
 * capacity.
 *
 * @param arena: pointer to a mem_arena
 * @param capacity: Maximum size in bytes that the arena can hold
 */
void arena_init(mem_arena *arena, uint64_t capacity);

/*
 * @brief: Frees a mem_arena
 *
 * @param arena: pointer to a mem_arena
 */
void arena_free(mem_arena *arena);

/*
 * @brief: Allocates memory from the arena.
 *
 * @param arena: Pointer to the arena to allocate from
 * @param size: Number of bytes to allocate
 *
 * @return: Pointer to allocated memory, or NULL if arena is full
 */
void *arena_push(mem_arena *arena, uint64_t size);

/*
 * @brief: Deallocates the most recently allocated memory from the arena.
 *
 * @param arena: Pointer to the arena to deallocate from
 * @param size: Number of bytes to deallocate from the top
 */
void arena_pop(mem_arena *arena, uint64_t size);

/*
 * @brief: Resets arena to a specific position, freeing all memory above it.
 *
 * @param arena: Pointer to the arena
 * @param pos: Position in bytes to reset the arena to
 */
void arena_pop_to(mem_arena *arena, uint64_t pos);

/*
 * @brief: Clears the entire arena, freeing all allocated memory.
 *
 * @param arena: Pointer to the arena to clear
 */
void arena_clear(mem_arena *arena);

/*
 * @brief: Helper macro to allocate and return a pointer to a struct of type T
 *
 * @param arena: Pointer to the arena to clear
 * @param T: any data-type
 */
#define arena_push_struct(arena, T) (T *)arena_push((arena), sizeof(T))

#endif // !ARENA_H
