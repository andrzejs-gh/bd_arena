#ifndef BD_ARENA_H
#define BD_ARENA_H

#include <stddef.h>

#define bda_DEFAULT_GF 1.0

#define bda_ALLOC(arena_ptr, type)                                      \
        bd_arena_alloc( (arena_ptr), sizeof(type), _Alignof(type) )

#define bda_HALLOC(arena_ptr, type)                                     \
        bd_arena_halloc( (arena_ptr), sizeof(type), _Alignof(type) )

#define bda_BUFFER(arena_ptr, size)                                     \
        bda_ALLOC((arena_ptr), char[size])

#define bda_HBUFFER(arena_ptr, size)                                    \
        bda_HALLOC((arena_ptr), char[size])

typedef struct bda_block_header bda_block_header;

typedef struct bda_block_header
{
    size_t free_space;
    bda_block_header* prev_block_handle;

} bda_block_header;

typedef struct
{
    size_t current_block_capacity;
    size_t total_size;
    double growth_factor;
    unsigned char* cursor;
    unsigned char* current_block_addr;

} bd_arena;

extern const bd_arena INVALID_ARENA;

bd_arena bd_arena_init(size_t block_capacity);
bd_arena* bd_arena_is_valid(bd_arena* arena);
void* bd_arena_alloc(bd_arena* arena, size_t size, size_t alignment);
void* bd_arena_halloc(bd_arena* arena, size_t size, size_t alignment);
void bd_arena_free(bd_arena* arena);

#endif
