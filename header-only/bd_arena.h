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
void* bda_alloc(bd_arena* arena, size_t size, size_t alignment);
void* bda_halloc(bd_arena* arena, size_t size, size_t alignment);
void* bda_put_bytes(bd_arena* arena, const void* buffer, size_t len);
void* bda_hput_bytes(bd_arena* arena, const void* buffer, size_t len);
char* bda_put_str(bd_arena* arena, const char* str);
char* bda_hput_str(bd_arena* arena, const char* str);
void bda_lock(bd_arena* arena);
void bda_free(bd_arena* arena);

#ifdef BD_ARENA_IMPLEMENTATION

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct bda_block_header bda_block_header;

typedef struct bda_block_header
{
    size_t free_space;
    bda_block_header* prev_block_handle;

} bda_block_header;

const bd_arena INVALID_ARENA = {0};

bd_arena bda_init(size_t block_capacity)
{
    bda_block_header* handle = malloc(sizeof *handle + block_capacity);
    if ( !handle ) return INVALID_ARENA;

    *handle = (bda_block_header){block_capacity, NULL};

    return (bd_arena)
    {
        .current_block_capacity = block_capacity,
        .total_size = sizeof *handle + block_capacity,
        .growth_factor = bda_DEFAULT_GF,
        .cursor = (unsigned char*)handle + sizeof *handle,
        .current_block_addr = (unsigned char*)handle
    };
}

bd_arena* bda_is_valid(bd_arena* arena)
{
    if ( !arena )
        return NULL;
    if ( !arena->current_block_capacity )
        return NULL;
    if ( arena->total_size < arena->current_block_capacity )
        return NULL;
    if ( !arena->growth_factor )
        return NULL;

    uintptr_t cursor = (uintptr_t)arena->cursor;
    uintptr_t data_begin = (uintptr_t)arena->current_block_addr +
    sizeof(bda_block_header);
    uintptr_t data_end = data_begin + arena->current_block_capacity;

    if ( cursor < data_begin || cursor >= data_end )
        return NULL;
    if ( !arena->current_block_addr )
        return NULL;

    return arena;
}

void* bda_alloc(bd_arena* arena, size_t size, size_t alignment)
{
    if ( !arena ) return NULL;

    if ( *(size_t*)(arena->current_block_addr) >= size )
    {
        uintptr_t old_cursor = (uintptr_t)arena->cursor;
        uintptr_t pos = (old_cursor + alignment - 1) & ~(alignment - 1);

        // updating free_space in the block header
        *(size_t*)(arena->current_block_addr) -= (size + (pos - old_cursor));

        arena->cursor = (unsigned char*)pos + size;
        return (unsigned char*)pos;
    }
    else
    {
        size_t new_block_capacity = arena->current_block_capacity
        *arena->growth_factor;

        if ( new_block_capacity < size )
            return NULL;

        size_t new_block_size = sizeof(bda_block_header) + new_block_capacity;

        bda_block_header* handle = malloc(new_block_size);
        if ( !handle )
            return NULL;

        uintptr_t data_block_begin = (uintptr_t)handle + sizeof *handle;
        uintptr_t pos = (data_block_begin + alignment - 1) & ~(alignment - 1);
        size_t taken_space = (size + (pos - data_block_begin));

        if ( taken_space > new_block_capacity )
        {
            free(handle);
            return NULL;
        }

        *handle = (bda_block_header)
                  {
                      new_block_capacity - taken_space,
                      (bda_block_header*)arena->current_block_addr
                  };

        arena->total_size += new_block_size;
        arena->current_block_capacity = new_block_capacity;
        arena->current_block_addr = (unsigned char*)handle;

        arena->cursor = (unsigned char*)pos + size;
        return (unsigned char*)pos;
    }
}

void* bda_halloc(bd_arena* arena, size_t size, size_t alignment)
{
    uintptr_t old_cursor = (uintptr_t)arena->cursor;
    uintptr_t pos = (old_cursor + alignment - 1) & ~(alignment - 1);

    // updating free_space in the block header
    *(size_t*)(arena->current_block_addr) -= (size + (pos - old_cursor));

    arena->cursor = (unsigned char*)pos + size;
    return (unsigned char*)pos;
}

void* bda_put_bytes(bd_arena* arena, const void* buffer, size_t len)
{
    void* ptr = bda_RESERVE(arena, len);
    if ( !ptr )
        return NULL;

    memcpy(ptr, buffer, len);
    return ptr;
}

void* bda_hput_bytes(bd_arena* arena, const void* buffer, size_t len)
{
    void* ptr = bda_HRESERVE(arena, len);

    memcpy(ptr, buffer, len);
    return ptr;
}

char* bda_put_str(bd_arena* arena, const char* str)
{
    if ( !str )
        return NULL;

    size_t len = strlen(str) + 1;

    char* ptr = bda_RESERVE(arena, len);
    if ( !ptr )
        return NULL;

    memcpy(ptr, str, len);
    return ptr;
}

char* bda_hput_str(bd_arena* arena, const char* str)
{
    size_t len = strlen(str) + 1;

    char* ptr = bda_HRESERVE(arena, len);

    memcpy(ptr, str, len);
    return ptr;
}

void bda_lock(bd_arena* arena)
{
    if ( !arena ) return;

    arena->growth_factor = 0.0;
}

void bda_free(bd_arena* arena)
{
    if ( !arena ) return;

    bda_block_header* block_handle = (bda_block_header*)arena->current_block_addr;
    bda_block_header* prev_block_handle;

    while ( block_handle )
    {
        prev_block_handle = block_handle->prev_block_handle;
        free(block_handle);
        block_handle = prev_block_handle;
    }

    *arena = INVALID_ARENA;
}

#endif

#endif
