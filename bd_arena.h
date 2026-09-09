#include <stddef.h>

#define bd_arena_DEFAULT_GF 1.0

#define bd_arena_ALLOC(arena_ptr, type)                             \
        bd_arena_alloc( (arena_ptr), sizeof(type), _Alignof(type) )

typedef struct bd_arena_handle bd_arena_handle;

typedef struct bd_arena_handle
{
    bd_arena_handle* prev_block_handle;

} bd_arena_handle;

typedef struct
{
    size_t current_block_capacity;
    size_t total_size;
    double growth_factor;
    unsigned char* cursor;
    unsigned char* current_block_addr;

} bd_arena;

extern const bd_arena INVALID_ARENA;
