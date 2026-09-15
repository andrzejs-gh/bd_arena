# BD_ARENA Documentation

## Table of contents

- [Overview](#overview)
- [API](#api)

---

## Overview
**BD_ARENA** is a dynamic arena implementation written in C.

Public (non-opaque) struct:
```c
typedef struct
{
    size_t current_block_capacity;
    size_t total_size;
    double growth_factor;
    unsigned char* cursor;
    unsigned char* current_block_addr;

} bd_arena;
```

### Usage
```c
/* Let's create new arena and give it a capacity of 1 KiB */
bd_arena arena = bda_init(1024); // bd_arena is always returned by value

/* Let's allocate some variables */
int* some_int = bda_ALLOC(&arena, int);
*some_int = -33;

double* some_double = bda_ALLOC(&arena, double);
*some_double = 0.1234567;

uint64_t* some_u64 = bda_ALLOC(&arena, uint64_t);
*some_u64 = 12345678131329;

/* Now let's allocate some strings and bytes */
const char* some_str = "some string that may be modified later";
char* some_str_cpy = bda_put_str(&arena, some_str);

uint8_t some_buffer[] = {0x7A, 0x0F, 0xC1, 0x00, 0xE5, 0x92, 0x4B, 0x07};
size_t some_buffer_len = sizeof(some_buffer) / sizeof(uint8_t);
uint8_t* some_buffer_ptr = bda_put_bytes(&arena, some_buffer, some_buffer_len);

/* Now lets exceed the 1 KiB capacity */
size_t space = bda_block_free_space(&arena);
printf("There is %zu bytes left. \n", space); // prints remaining free space
                                              // in the current block

printf(
          "Total occupied size (all blocks + headers) = %zu \n",
          arena.total_size
      );  // prints total space occupied by the arena 

struct some_struct
{
    uint8_t buffer[1024];  // this easily exceeds the current block
                           // which already contains data (potentially padded)
};

struct some_struct* ptr = bda_ALLOC(&arena, struct some_struct);
// Because the requested space exceeded the current block capacity,
// arena had to grow by allocating a new block. The new block's size
// is determined by arena's growth_factor:
// new_block_capacity = old_block_capacity * growth_factor
// We didn't change the growth_factor value (default value is 1.0),
// so the new block is the same size as the initial block.

printf("There is %zu bytes left. \n", bda_block_free_space(&arena));
// prints 0 because the second block is entirely occupied
// with a struct equal to its size

printf(
          "Total occupied size (all blocks + headers) = %zu \n",
          arena.total_size
      );
// prints 2*(sizeof(bda_block_header) + 1024),
// bda_block_header is a small struct:
//
// typedef struct bda_block_header
// {
//     size_t free_space;
//     bda_block_header* prev_block_handle;
//
// } bda_block_header;

/* if we try to allocate something larger than
   current_block_capacity * growth_factor,
   the allocator will fail and return NULL */
int* large_array = bda_ALLOC(&arena, int[1000]);   // NULL
void* reserved_buffer = bda_RESERVE(&arena, 2000); // NULL
// only hazardous allocator (bda_HALLOC, bda_HRESERVE) would
// have bypassed safety checks, corrupting memory in the process

/* but if we create a new block that's large enough,
  the allocation will succeed */
bda_new_block(&arena, 2000);
large_array = bda_ALLOC(&arena, int[1000]);
// alternatively, we could also tweak the growth_factor
// to make the arena grow sufficiently large block

/* We can also lock the arena which prevents it from growing */
bda_lock(&arena);
// this is equivalent to setting the growth_factor to 0

/* now let's free the arena */
bda_free(&arena);

/* the arena is now set to invalid state
d efined as: (bd_arena){0} */
if ( !bda_is_valid(&arena) )
    puts("Arena is in invalid state.");
```

<p align="right">
<a href="#table-of-contents">GO TO TOP ^</a>
</p>

---

## API

### Creation and destruction

* [bda_init](#-bda_init-)
* [bda_free](#-bda_free-)

### Validation

* [bda_is_valid](#-bda_is_valid-)

### Allocation (safe)

* [bda_ALLOC](#-bda_alloc-)
* [bda_RESERVE](#-bda_reserve-)
* [bda_put_str](#-bda_put_str-)
* [bda_put_bytes](#-bda_put_bytes-)

### Allocation (hazardous)

* [bda_HALLOC](#-bda_halloc-)
* [bda_HRESERVE](#-bda_hreserve-)
* [bda_hput_str](#-bda_hput_str-)
* [bda_hput_bytes](#-bda_hput_bytes-)

### Arena control

* [bda_new_block](#-bda_new_block-)
* [bda_lock](#-bda_lock-)

### Information

* [bda_block_free_space](#-bda_block_free_space-)

<p align="right">
<a href="#table-of-contents">GO TO TOP ^</a>
</p>

---

## Creation and destruction

### ** **bda_init** **

```c
bd_arena bda_init(size_t block_capacity);
```

Creates a new arena with requested capacity using `malloc`. The memory layout is:
```
... [ block header ] [ capacity-sized data block ] ...
```

The block header is an opaque struct defined as:
```c
// bd_arena.c

typedef struct bda_block_header
{
    size_t free_space;
    bda_block_header* prev_block_handle;

} bda_block_header;
```

In case of a failure, the function returns `INVALID_ARENA` defined as:
```c
const bd_arena INVALID_ARENA = (bd_arena){0}
```

Example:
```c
bd_arena arena = bda_init(1024); // creating 1 KiB arena
```

* **Return (success):**

  * `bd_arena` new arena instance

* **Return (failure):**

  * `INVALID_ARENA`
    - `block_capacity == 0`
    - allocation failure

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_free** **

```c
void bda_free(bd_arena* arena);
```

Frees all the blocks allocated by an arena. If `arena == NULL`, it returns without doing anything.

* **Return (success):**

  * None
  
* **Return (failure):**

  * None

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

## Validation

### ** **bda_is_valid** **

```c
bd_arena* bda_is_valid(bd_arena* arena);
```

Checks if an arena is valid. Returns back pointer to the arena if it's valid, otherwise it returns `NULL`

* **Return (success):**

  * `bd_arena*` arena pointer
  
* **Return (failure):**

  * `NULL`
    - `arena == NULL`
    - `arena->current_block_capacity == 0`
    - `arena->total_size < arena->current_block_capacity + sizeof(bda_block_header)`
    - `arena->growth_factor < 0.0`
    - `arena->cursor` points to the outside of the current block
    - `arena->current_block_addr == NULL`

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

## Allocation

### ** **bda_ALLOC** **

```c
bda_ALLOC(arena_ptr, type)
```

Primary allocator for puting objects of arbitrary types in the arena. Returns pointer to the buffer.

It guarantees the correct alignment and protects memory from corruption by growing a new block if an object can't fit in the current block. 

If the object's size (+ potential alignment offset) exceeds: 

`arena_ptr->current_block_capacity * arena_ptr->growth_factor`

the allocation fails and `NULL` is returned.

Examples:
```c
int* some_int = bda_ALLOC(arena_ptr, int);
*some_int = -33;

double* some_double = bda_ALLOC(arena_ptr, double);
*some_double = 0.1234567;

uint64_t* some_u64 = bda_ALLOC(arena_ptr, uint64_t);
*some_u64 = 12345678131329;
```

* **Return (success):**

  * `void*` pointer to the buffer where the object can be stored
  
* **Return (failure):**

  * `NULL`
    - `arena_ptr == NULL`
    - `arena->current_block_capacity * arena->growth_factor` produces insufficient size
    - allocation failure

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_HALLOC** **

```c
bda_HALLOC(arena_ptr, type)
```

Hazardous version of [bda_ALLOC](#-bda_alloc-).

It guarantees the correct alignment but **DOES NOT** protect the memory from corruption by growing a new block if an object can't fit in the current block. 

It provides no safety mechanisms, and that includes no check if `arena_ptr` is valid. Use with caution.

* **Return (success):**

  * `void*` pointer to the buffer where the object can be stored
  
* **Return (failure):**

  * None

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_RESERVE** **

```c
bda_RESERVE(arena_ptr, size)
```

Reserves a buffer of the given size. Equivalent to calling:
```c
bda_ALLOC(arena_ptr, char[size]);
```
see [bda_ALLOC](#-bda_alloc-).

Example:
```c
void* p = bda_RESERVE(arena_ptr, size);
```

* **Return (success):**

  * `void*` pointer to the buffer
  
* **Return (failure):**

  * `NULL`
    - `arena_ptr == NULL`
    - `arena->current_block_capacity * arena->growth_factor` produces insufficient size
    - allocation failure

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_HRESERVE** **

```c
bda_HRESERVE(arena_ptr, size)
```

Same as [bda_RESERVE](#-bda_reserve-) but hazardous. Equivalent to calling:
```c
bda_HALLOC(arena_ptr, char[size]);
```
see [bda_HALLOC](#-bda_halloc-).

Example:
```c
uint8_t* p = bda_HRESERVE(arena_ptr, size);
```

* **Return (success):**

  * `void*` pointer to the buffer

* **Return (failure):**

  * None

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_put_str** **

```c
char* bda_put_str(bd_arena* arena, const char* str);
```

Puts a string in the arena and returns pointer to it. Allocates a new block if neccessery.

Example:
```c
char* p = bda_put_str(arena_ptr, "to be copied and stored in the arena");
```

* **Return (success):**

  * `char*` pointer to the allocated string
  
* **Return (failure):**

  * `NULL`
    - `arena == NULL`
    - `str == NULL`
    - `arena->current_block_capacity * arena->growth_factor` produces insufficient size
    - allocation failure

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_hput_str** **

```c
char* bda_hput_str(bd_arena* arena, const char* str);
```

Hazardous version of [bda_put_str](#-bda_put_str-).

No safety checks and no memory protection. Use with caution.

* **Return (success):**

  * `char*` pointer to the allocated string
  
* **Return (failure):**

  * None

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_put_bytes** **

```c
void* bda_put_bytes(bd_arena* arena, const void* buffer, size_t len);
```

Puts a buffer of a given length in the arena and returns pointer to it. Allocates a new block if neccessery.

Example:
```c
void* p = bda_put_bytes(arena_ptr, buffer_ptr, buffer_len);
```

* **Return (success):**

  * `void*` pointer to the allocated buffer
  
* **Return (failure):**

  * `NULL`
    - `arena == NULL`
    - `buffer == NULL`
    - `len == 0`
    - `arena->current_block_capacity * arena->growth_factor` produces insufficient size
    - allocation failure

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_hput_bytes** **

```c
void* bda_hput_bytes(bd_arena* arena, const void* buffer, size_t len);
```

Hazardous version of [bda_put_bytes](#-bda_put_bytes-).

No safety checks and no memory protection. Use with caution.

* **Return (success):**

  * `void*` pointer to the allocated buffer

* **Return (failure):**

  * None

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

## Arena control

### ** **bda_new_block** **

```c
size_t bda_new_block(bd_arena* arena, size_t capacity);
```

Creates new memory block the size of a given capacity. On success it returns back the `capacity`, on failure it rteurns `0`.

* **Return (success):**

  * `size_t` capacity

* **Return (failure):**

  * `0`
    - `arena == NULL`
    - `capacity == 0`
    - allocation failure

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

### ** **bda_lock** **

```c
void bda_lock(bd_arena* arena);
```

Sets `arena->growth_factor` to `0.0` effectivelly locking the arena from growing further. If `arena == NULL`, the function returns without doing anything.

* **Return (success):**

  * None
  
* **Return (failure):**

  * None

<p align="right">
<a href="#api">GO TO API ^</a>
</p>

---

## Information

### ** **bda_block_free_space** **

```c
size_t bda_block_free_space(bd_arena* arena);
```

Returns the remaining free space in the current block. If `arena == NULL`, the function returns `0`.

* **Return (success):**

  * `size_t` remaining free space in the current block
  
* **Return (failure):**

  * `0`

<p align="right">
<a href="#api">GO TO API ^</a>
</p>
  
---
