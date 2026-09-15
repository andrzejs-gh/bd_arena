#define BD_ARENA_IMPLEMENTATION
#include "../header-only/bd_arena.h"

#include "time_exec.h"

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>

#define RED(x) "\033[31m" x "\033[0m"
#define GREEN(x) "\033[32m" x "\033[0m"
#define YELLOW(x) "\033[33m" x "\033[0m"
#define BLUE(x) "\033[34m" x "\033[0m"
#define CYAN(x) "\033[36m" x "\033[0m"

#define OK "[ " GREEN("OK") " ]"
#define ERROR "[ " RED("ERROR") " ]"

#define CR "\033[G"
#define CLEAR_LINE "\033[2K\033[G"

#define KiB 1024
#define MiB 1024*KiB

#define LIMIT 1000000

static inline uint64_t random_from_range(uint64_t min, uint64_t max)
{
    return min + random() % (max - min);
}

void allocation_time_test(void)
{
    double time;
    uint64_t i = 0;

    bd_arena arena_one = bda_init(8);
    arena_one.growth_factor = 2.00;

    bd_arena arena_two = bda_init(LIMIT*sizeof(uint64_t));
    bd_arena arena_three = bda_init(2*LIMIT*sizeof(uint64_t));

    printf(YELLOW("Allocating %d uint64_t's: \n"), LIMIT);

    puts(BLUE("* * *"));

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = bda_ALLOC(&arena_one, uint64_t);
        *p = i++;
    );
    i = 0;

    printf("bda_ALLOC:  t = %.9f\n", time);
    printf
    (
        "arena_one total_size = %zu \n"
        "arena_one init. cap. was only 8, growth_factor = %.2f \n",
        arena_one.total_size,
        arena_one.growth_factor
    );

    puts(BLUE("* * *"));

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = bda_ALLOC(&arena_two, uint64_t);
        *p = i++;
    );
    i = 0;

    printf("bda_ALLOC:  t = %.9f\n", time);
    printf
    (
        "arena_two total_size = %zu \n"
        "arena_two init. cap. was %zu \n",
        arena_two.total_size,
        LIMIT*sizeof(uint64_t)
    );

    puts(BLUE("* * *"));

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = bda_HALLOC(&arena_three, uint64_t);
        *p = i++;
    );
    i = 0;

    printf("bda_HALLOC: t = %.9f\n", time);
    printf
    (
        "arena_three total_size = %zu \n"
        "arena_three init. cap. was %zu \n",
        arena_three.total_size,
        2*LIMIT*sizeof(uint64_t)
    );

    puts(BLUE("* * *"));

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = malloc(sizeof *p);   // it will create an 8 MB memory leak
        *p = i++;                          // but that's fine for the test purposes
    );

    printf("malloc:     t = %.9f \n", time);

    puts(BLUE("* * *"));

    puts(CYAN("================================"));

    bda_free(&arena_one);   //free(arena_one_ptrs);
    bda_free(&arena_two);   //free(arena_two_ptrs);
    bda_free(&arena_three); //free(arena_three_ptrs);
}

void put_test(void)
{
    puts(YELLOW("Insertion test:"));

    char buffer[256];
    puts("Insert arbitrary (max 255 bytes) text and press ENTER:");

    void* ret = fgets(buffer, sizeof(buffer), stdin);
    if ( !ret )
    {
        puts(ERROR " fgets failure");
        assert(false);
    }

    bd_arena ar = bda_init(sizeof(buffer));
    char* p = bda_put_str(&ar, buffer);
    printf(GREEN("Inserted text:") "\n%s", p);

    bda_free(&ar);

    puts("Press any key to continue...");
    getchar();
    fflush(stdin);

    puts(CYAN("================================"));
}

void arena_randomized_test(void)
{
    puts(YELLOW("Randomized test..."));

    srandom((unsigned)time(NULL));
    uint64_t* p;

    bd_arena alloc_arena_small = bda_init(8);
    alloc_arena_small.growth_factor = 1.00;

    bd_arena alloc_arena = bda_init(LIMIT*sizeof(uint64_t));
    bd_arena halloc_arena = bda_init(2*LIMIT*sizeof(uint64_t));

    uint64_t** alloc_arena_small_ptrs = malloc(LIMIT*sizeof(uint64_t*));
    uint64_t** alloc_arena_ptrs = malloc(LIMIT*sizeof(uint64_t*));
    uint64_t** halloc_arena_ptrs = malloc(LIMIT*sizeof(uint64_t*));

    for ( uint64_t i = 0; i < LIMIT; i++ )
    {
        p = bda_ALLOC(&alloc_arena_small, uint64_t);
        *p = i;
        alloc_arena_small_ptrs[i] = p;

        p = bda_ALLOC(&alloc_arena, uint64_t);
        *p = i;
        alloc_arena_ptrs[i] = p;

        p = bda_HALLOC(&halloc_arena, uint64_t);
        *p = i;
        halloc_arena_ptrs[i] = p;
    }

    uint64_t iters = LIMIT*10;

    for ( uint64_t i = 0; i < iters; i++ )
    {
        uint64_t index = random_from_range(0, LIMIT);

        switch ( index % 3 )
        {
            case 0:
                if ( *alloc_arena_small_ptrs[index] != index )
                {
                    printf("alloc_arena_small: data mismatch at %zu\n", index);
                    printf("expected value: %zu, actual value: %zu",
                            index, *alloc_arena_small_ptrs[index]);
                    assert(false);
                } break;

            case 1:
                if ( *alloc_arena_ptrs[index] != index )
                {
                    printf("alloc_arena: data mismatch at %zu\n", index);
                    printf("expected value: %zu, actual value: %zu",
                           index, *alloc_arena_ptrs[index]);
                    assert(false);
                } break;

            case 2:
                if ( *halloc_arena_ptrs[index] != index )
                {
                    printf("halloc_arena: data mismatch at %zu\n", index);
                    printf("expected value: %zu, actual value: %zu",
                           index, *halloc_arena_ptrs[index]);
                    assert(false);
                } break;
        }
        printf(CR);
        printf("[ %.2f %% ]", ((double)(i+1) / iters) * 100);
        fflush(stdout);
    }

    bda_free(&alloc_arena_small); free(alloc_arena_small_ptrs);
    bda_free(&alloc_arena);       free(alloc_arena_ptrs);
    bda_free(&halloc_arena);      free(halloc_arena_ptrs);

    puts(OK " Randomized test passed");
    puts(BLUE("* * *"));
    puts(CYAN("================================"));
}

void readme_scenario(void)
{
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
     c urrent_block_capacity * growth_fact*or,
     the allocator will fail and return NULL */
    int* large_array = bda_ALLOC(&arena, int[1000]);   // NULL
    void* reserved_buffer = bda_RESERVE(&arena, 2000); // NULL
    // only hazardous allocator (bda_HALLOC, bda_HRESERVE) would
    // have bypassed safety checks, corrupting memory in the process

    /* but if we create a new block that's large enough,
     t he allocation will succeed */
     bda_new_block(&arena, 2000);
     large_array = bda_ALLOC(&arena, int[1000]);
     // alternatively, we could also tweak the growth_factor
     // to make the arena grow sufficiently large block

     /* now let's free the arena */
     bda_free(&arena);

     /* the arena is now set to invalid state
      d efined as: (bd_arena){0} */
      if ( !bda_is_valid(&arena) )
          puts("Arena is in invalid state.");
}

int main(void)
{
    put_test();
    allocation_time_test();
    arena_randomized_test();

    return 0;
}
