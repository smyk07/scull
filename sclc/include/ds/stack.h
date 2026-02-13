#ifndef STACK
#define STACK

#include "common.h"

typedef struct stack {
  void *items;
  u64 item_size;
  u64 count;
  u64 capacity;
} stack;

#define STACK_INITIAL_CAPACITY 4
#define STACK_RESIZE_FACTOR 2

u32 stack_init(stack *s, u64 item_size);

u32 stack_push(stack *s, void *item);

u32 stack_pop(stack *s, void *item);

void *stack_top(stack *s);

void stack_free(stack *s);

#endif // !STACK
