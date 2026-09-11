#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "bd_arena.h"

const bd_arena INVALID_ARENA = {0};

bd_arena bd_arena_init(size_t block_capacity)
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

bd_arena* bd_arena_is_valid(bd_arena* arena)
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

void* bd_arena_alloc(bd_arena* arena, size_t size, size_t alignment)
{
    if ( !arena ) return NULL;

    if ( *(size_t*)(arena->current_block_addr) >= size )
    {
        unsigned char* ptr = arena->cursor;

        size_t offset = (uintptr_t)ptr % alignment;
        if ( offset )
        {
            ptr = ptr + alignment - offset;
            *(size_t*)(arena->current_block_addr) = arena->current_block_capacity
                                                    - (size + alignment - offset);
        }
        else
        {
            *(size_t*)(arena->current_block_addr) = arena->current_block_capacity
                                                                           - size;
        }

        arena->cursor = ptr + size;
        return ptr;
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

        *handle = (bda_block_header)
                  {
                      new_block_capacity,
                      (bda_block_header*)arena->current_block_addr
                  };

        arena->total_size += new_block_size;
        arena->current_block_capacity = new_block_capacity;
        arena->current_block_addr = (unsigned char*)handle;

        unsigned char* ptr = (unsigned char*)handle + sizeof *handle;
        size_t offset = (uintptr_t)ptr % alignment;
        if ( offset )
        {
            ptr = ptr + alignment - offset;
            handle->free_space = new_block_capacity - (size + alignment - offset);
        }
        else
        {
            handle->free_space = new_block_capacity - size;
        }

        arena->cursor = ptr + size;
        return ptr;
    }
}

void* bd_arena_halloc(bd_arena* arena, size_t size, size_t alignment)
{
    unsigned char* ptr = arena->cursor;
    size_t offset = (uintptr_t)ptr % alignment;
    if ( offset )
        ptr = ptr + alignment - offset;

    arena->cursor = ptr + size;
    return ptr;
}

// char* bd_arena_alloc_str(bd_arena* arena, char* str)
// {
//     size_t buffer_len = strlen(str) + 1;
//
//     if
//     (
//         (uintptr_t)arena->cursor + buffer_len >
//         (uintptr_t)arena->current_block_addr +
//         sizeof(bda_block_header) +
//         arena->current_block_capacity
//
//     )   {
//             size_t new_block_capacity = arena->current_block_capacity
//                                                 *arena->growth_factor;
//
//
//
//             size_t new_block_size = sizeof(bda_block_header) + new_block_capacity;
//
//             bda_block_header* handle = malloc(new_block_size);
//             if ( !handle ) return NULL;
//
//             unsigned char* new_cursor = (unsigned char*)handle + sizeof *handle;
//
//             if
//                 (
//                     (ptr - (unsigned char*)handle) + size >
//                     sizeof *handle + arena->current_block_capacity
//
//                 ) { free(handle); return NULL; }
//
//             arena->total_size += new_block_size;
//             arena->current_block_capacity = new_block_capacity;
//             handle->prev_block_handle = (bda_block_header*)arena->current_block_addr;
//             arena->current_block_addr = (unsigned char*)handle;
//
//
//         }
//
//     // memcpy(arena->cursor, str, buffer_len);
//
//     return str;
// }

void bd_arena_free(bd_arena* arena)
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
