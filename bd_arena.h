#ifndef BD_ARENA_H
#define BD_ARENA_H

#include <stddef.h>

#define bda_DEFAULT_GF 1.0

#define bda_ALLOC(arena_ptr, type)                                          \
        bda_alloc( (arena_ptr), sizeof(type), _Alignof(type) )

#define bda_HALLOC(arena_ptr, type)                                         \
        bda_halloc( (arena_ptr), sizeof(type), _Alignof(type) )

#define bda_RESERVE(arena_ptr, size)                                        \
        bda_alloc( (arena_ptr), (size), 1 )

#define bda_HRESERVE(arena_ptr, size)                                       \
        bda_halloc( (arena_ptr), (size), 1 )

typedef struct
{
    size_t current_block_capacity;
    size_t total_size;
    double growth_factor;
    unsigned char* cursor;
    unsigned char* current_block_addr;

} bd_arena;

extern const bd_arena INVALID_ARENA;

bd_arena bda_init(size_t block_capacity);
bd_arena* bda_is_valid(bd_arena* arena);
size_t bda_block_free_space(bd_arena* arena);
size_t bda_new_block(bd_arena* arena, size_t capacity);
void* bda_alloc(bd_arena* arena, size_t size, size_t alignment);
void* bda_halloc(bd_arena* arena, size_t size, size_t alignment);
void* bda_put_bytes(bd_arena* arena, const void* buffer, size_t len);
void* bda_hput_bytes(bd_arena* arena, const void* buffer, size_t len);
char* bda_put_str(bd_arena* arena, const char* str);
char* bda_hput_str(bd_arena* arena, const char* str);
void bda_lock(bd_arena* arena);
void bda_free(bd_arena* arena);

#endif
