#include "../bd_arena.h"
#include "../bd_arena.c"

#define KiB 1024
#define MiB 1024*KiB

void test(bd_arena* arena)
{

}

int main(void)
{
    bd_arena test_arena = bd_arena_init(KiB);

    return 0;
}
