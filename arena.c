#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
    unsigned char* prev_block_ptr;

} bd_arena_handle;

typedef struct
{
    size_t block_capacity;
    size_t total_size;
    unsigned char* cursor;
    unsigned char* current_block_addr;

} bd_arena;

const bd_arena INVALID_ARENA = {0};

bd_arena bd_arena_init(size_t block_capacity)
{
    bd_arena_handle* handle = malloc(sizeof *handle + block_capacity);
    if ( !handle ) return INVALID_ARENA;

    handle->prev_block_ptr = NULL;

    return (bd_arena)
           {
               .block_capacity = block_capacity,
               .total_size = sizeof *handle + block_capacity,
               .cursor = (unsigned char*)handle + sizeof *handle,
               .current_block_addr = (unsigned char*)handle
           };
}

static void* bd_arena_alloc(bd_arena* arena, size_t size, size_t alignment)
{
    unsigned char* cursor = arena->cursor;
    size_t offset = (uintptr_t)cursor % alignment;
    unsigned char* new_cursor = cursor + alignment - offset;

    if
    (
        (new_cursor - arena->current_block_addr) + size >
        sizeof(bd_arena_handle) + arena->block_capacity

    ) {
         // if (  )

         bd_arena_handle* handle = malloc(sizeof *handle + arena->block_capacity);
         if ( !handle ) return NULL;

         arena->total_size += sizeof *handle + arena->block_capacity;
         handle->prev_block_ptr = arena->current_block_addr;

         new_cursor = (unsigned char*)handle + sizeof *handle;

      }

    arena->cursor = new_cursor;
    return new_cursor;
}

// void* bd_arena_alloc(bd_arena* arena, void* data, size_t data_size, size_t alignment)
// {
//     unsigned char* ptr = get_next_position(arena->cursor, alignment);
//     memcpy(ptr, data, data_size);
//
//     arena->cursor = ptr;
//     return ptr;
// }
