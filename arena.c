#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define bda_OFFSET  ( (bd_arena)->cursor % _Alignof(type) )
#define bda_NEW_POS ( (bd_arena)->cursor + _Alignof(type) - bda_OFFSET )
// #define bda_VALID

#define bd_arena_PUT(bd_arena, type, ptr)   \
                    (ptr) =  ?            \
                    (                       \
                                            \
                                            \
                    ) :

typedef struct
{
    size_t capacity;
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
               .capacity = capacity,
               .cursor = addr,
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

void* bd_arena_alloc(bd_arena* arena, void* data, size_t data_size, size_t alignment)
{
    unsigned char* ptr = get_next_position(arena->cursor, alignment);
    memcpy(ptr, data, data_size);

    arena->cursor = ptr;
    return ptr;
}
