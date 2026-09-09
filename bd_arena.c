#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "bd_arena.h"

const bd_arena INVALID_ARENA = {0};

bd_arena bd_arena_init(size_t block_capacity)
{
    bd_arena_handle* handle = malloc(sizeof *handle + block_capacity);
    if ( !handle ) return INVALID_ARENA;

    handle->prev_block_handle = NULL;

    return (bd_arena)
           {
               .current_block_capacity = block_capacity,
               .total_size = sizeof *handle + block_capacity,
               .growth_factor = bd_arena_DEFAULT_GF,
               .cursor = (unsigned char*)handle + sizeof *handle,
               .current_block_addr = (unsigned char*)handle
           };
}

void* bd_arena_alloc(bd_arena* arena, size_t size, size_t alignment)
{
    if ( !arena ) return NULL;

    unsigned char* new_cursor = arena->cursor;
    size_t offset = (uintptr_t)new_cursor % alignment;
    if ( offset )
        new_cursor = new_cursor + alignment - offset;

    if
    (
        (new_cursor - arena->current_block_addr) + size >
        sizeof(bd_arena_handle) + arena->current_block_capacity

    ) {
            size_t new_block_capacity = arena->current_block_capacity
                                                *arena->growth_factor;
            //arena->current_block_capacity *= arena->growth_factor;
            size_t new_block_size = sizeof(bd_arena_handle) + new_block_capacity;

            bd_arena_handle* handle = malloc(new_block_size);
            if ( !handle ) return NULL;

            new_cursor = (unsigned char*)handle + sizeof *handle;
            offset = (uintptr_t)new_cursor % alignment;
            if ( offset )
                new_cursor = new_cursor + alignment - offset;

            if
            (
                (new_cursor - (unsigned char*)handle) + size >
                sizeof *handle + arena->current_block_capacity

            ) { free(handle); return NULL; }

            arena->total_size += new_block_size;
            arena->current_block_capacity = new_block_capacity;
            handle->prev_block_handle = (bd_arena_handle*)arena->current_block_addr;
            arena->current_block_addr = (unsigned char*)handle;
      }

    arena->cursor = new_cursor;
    return new_cursor;
}

void bd_arena_free(bd_arena* arena)
{
    bd_arena_handle* block_handle = (bd_arena_handle*)arena->current_block_addr;
    bd_arena_handle* prev_block_handle;

    while ( block_handle )
    {
        prev_block_handle = block_handle->prev_block_handle;
        free(block_handle);
        block_handle = prev_block_handle;
    }

    *arena = INVALID_ARENA;
}
