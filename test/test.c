#include "../bd_arena.h"
#include "../bd_arena.c"
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
    arena_one.growth_factor = 1.00;

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

int main(void)
{
    put_test();
    allocation_time_test();
    arena_randomized_test();

    return 0;
}
