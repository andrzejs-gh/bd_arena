#include "../bd_arena.h"
#include "../bd_arena.c"
#include "time_exec.h"

#include <stdint.h>

#define RED(x) "\033[31m" x "\033[0m"
#define GREEN(x) "\033[32m" x "\033[0m"
#define YELLOW(x) "\033[33m" x "\033[0m"
#define CYAN(x) "\033[36m" x "\033[0m"

#define OK "[ " GREEN("OK") " ]"
#define ERROR "[ " RED("ERROR") " ]"

#define CR "\033[G"
#define CLEAR_LINE "\033[2K\033[G"

#define KiB 1024
#define MiB 1024*KiB

#define LIMIT 1000000

void allocation_test(void)
{
    double time;
    uintptr_t i = 0;

    bd_arena arena_one = bd_arena_init(8);
    arena_one.growth_factor = 1.00;

    bd_arena arena_two = bd_arena_init(LIMIT*sizeof(uint64_t));
    bd_arena arena_three = bd_arena_init(2*LIMIT*sizeof(uint64_t));

    printf(YELLOW("Allocating %d uint64_t's: \n"), LIMIT);

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = bda_ALLOC(&arena_one, uint64_t);
        *p = i++;
    );

    printf
    (
        "bda_ALLOC: t = %.9f" " (arena initialized as only 8 B, growth factor = %.2f)"
        "\n",
        time,
        arena_one.growth_factor
    ); printf("arena_one total_size = %zu \n", arena_one.total_size);
    i = 0;

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = bda_ALLOC(&arena_two, uint64_t);
        *p = i++;
    );

    printf
    (
        "bda_ALLOC: t = %.9f" " (arena initialized as %d*%zu B)"
        "\n",
        time,
        LIMIT, sizeof(uint64_t)
    ); printf("arena_two total_size = %zu \n", arena_two.total_size);
    i = 0;

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = bda_HALLOC(&arena_three, uint64_t);
        *p = i++;
    );

    printf
    (
        "bda_HALLOC: t = %.9f" " (arena initialized as %d*%zu B)"
        "\n",
        time,
        2*LIMIT, sizeof(uint64_t)
    ); printf("arena_three total_size = %zu \n", arena_three.total_size);
    i = 0;

    BENCH_STORE
    (
        0,
        LIMIT,
        time,

        uint64_t* p = malloc(sizeof *p); // this will end up in 8 MB
        *p = i++;                        // memory leak, but that's ok
    );                                   // for the test purposes

    printf("malloc:    t = %.9f \n", time);

    puts(CYAN("================================"));

    bd_arena_free(&arena_one);
    bd_arena_free(&arena_two);
    bd_arena_free(&arena_three);
}

int main(void)
{
    allocation_test();

    bd_arena test_arena = bd_arena_init(KiB);
    //if test_arena

    bd_arena_free(&test_arena);

    return 0;
}
