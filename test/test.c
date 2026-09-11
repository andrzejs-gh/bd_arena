#include "../bd_arena.h"
#include "../bd_arena.c"
#include "time_exec.h"

#include <stdint.h>

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

void allocation_test(void)
{
    double time;
    uintptr_t i = 0;

    bd_arena arena_one = bd_arena_init(8);
    arena_one.growth_factor = 1.00;

    bd_arena arena_two = bd_arena_init(LIMIT*sizeof(uint64_t));
    bd_arena arena_three = bd_arena_init(2*LIMIT*sizeof(uint64_t));

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

        uint64_t* p = malloc(sizeof *p); // this will end up in 8 MB
        *p = i++;                        // memory leak, but that's ok
    );                                   // for the test purposes

    printf("malloc:     t = %.9f \n", time);

    puts(BLUE("* * *"));

    puts(CYAN("================================"));

    bd_arena_free(&arena_one);
    bd_arena_free(&arena_two);
    bd_arena_free(&arena_three);
}

void buff_alloc(void)
{

    const char* t = "Sławuś bławuś elemelek chujek ejwdiewbfkebfewf";
    const char* tt = "weoiew fiew fiew fuhe wify ewifybef iewyf u ejwdiewbfkebfewf";

    bd_arena arena = bd_arena_init(strlen(tt)+1);
    printf("arena init size = %zu \n", arena.total_size);

    char* p = bda_PUT_STR(&arena, t);
    puts(p);

    char* pp = bda_PUT_BYTES(&arena, t, strlen(t)+1);
    puts(pp);

    p = bda_PUT_STR(&arena, tt);
    puts(p);

    pp = bda_PUT_BYTES(&arena, tt, strlen(tt)+1);
    puts(pp);

    p = bda_PUT_STR(&arena, "chuje muje dzikie wenżę");
    puts(p);

    printf("arena total size = %zu \n", arena.total_size);

    bd_arena_free(&arena);
}

int main(void)
{
    allocation_test();
    buff_alloc();

    bd_arena test_arena = bd_arena_init(KiB);
    //if test_arena

    bd_arena_free(&test_arena);

    return 0;
}
