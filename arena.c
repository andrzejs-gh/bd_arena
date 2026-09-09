#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
    size_t block_capacity;
    size_t total_size;
    unsigned char* cursor;
    unsigned char* addr;

} bd_arena;

const bd_arena INVALID_ARENA = {0};

bd_arena bd_arena_init(size_t capacity)
{
    unsigned char* addr = malloc(capacity);
    if ( !addr ) return INVALID_ARENA;

    return (bd_arena)
           {
               .block_capacity = capacity,
               .total_size = capacity,
               .cursor = addr + sizeof(void*),
               .addr = addr
           };
}

static inline void* get_next_position(void* cursor, size_t alignment)
{
    size_t offset = (uintptr_t)cursor % alignment;
    if ( !offset )
        return cursor;

    return cursor + alignment - offset;
}

// void* bd_arena_alloc(bd_arena* arena, void* data, size_t data_size, size_t alignment)
// {
//     unsigned char* ptr = get_next_position(arena->cursor, alignment);
//     memcpy(ptr, data, data_size);
//
//     arena->cursor = ptr;
//     return ptr;
// }
