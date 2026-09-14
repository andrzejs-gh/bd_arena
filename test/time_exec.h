#ifndef MTLK_TIME_EXECUTION_H
#define MTLK_TIME_EXECUTION_H

#include <time.h>
#include <stdio.h>

#define GET_EXEC_TIME_(func_or_code) \
do { \
    struct timespec T_EXEC_MACRO_start_ts, T_EXEC_MACRO_end_ts; \
    clock_gettime(CLOCK_MONOTONIC, &T_EXEC_MACRO_start_ts); \
    func_or_code;  \
    clock_gettime(CLOCK_MONOTONIC, &T_EXEC_MACRO_end_ts); \
    double T_EXEC_MACRO_start = T_EXEC_MACRO_start_ts.tv_sec + \
                                T_EXEC_MACRO_start_ts.tv_nsec * 1e-9; \
    double T_EXEC_MACRO_end = T_EXEC_MACRO_end_ts.tv_sec + \
                              T_EXEC_MACRO_end_ts.tv_nsec * 1e-9;

#define TIME_PRINT(func_or_code) \
    GET_EXEC_TIME_(func_or_code) \
    printf("t = %.9f s\n", T_EXEC_MACRO_end - \
                           T_EXEC_MACRO_start); \
} while (0);

#define TIME_STORE(variable, func_or_code) \
    GET_EXEC_TIME_(func_or_code) \
    variable = T_EXEC_MACRO_end - \
               T_EXEC_MACRO_start; \
} while (0);

#define REPEAT(N, func_or_code) \
do { \
    for (size_t T_EXEC_MACRO_i = 0; T_EXEC_MACRO_i < N; T_EXEC_MACRO_i++) \
    { \
        func_or_code; \
    } \
} while (0);

#define BENCHMARK_(WARM_UP, N, func_or_code) \
do { \
    double T_EXEC_MACRO_total_time; \
    REPEAT(WARM_UP, func_or_code) \
    TIME_STORE( \
        T_EXEC_MACRO_total_time, \
        REPEAT(N, func_or_code) \
    ) \
    double T_EXEC_MACRO_avg_time = T_EXEC_MACRO_total_time / N;

#define BENCH_PRINT(WARM_UP, N, func_or_code) \
    BENCHMARK_(WARM_UP, N, func_or_code) \
    printf("t = %.9f s\n", T_EXEC_MACRO_avg_time); \
} while (0);

#define BENCH_STORE(WARM_UP, N, variable, func_or_code) \
    BENCHMARK_(WARM_UP, N, func_or_code) \
    variable = T_EXEC_MACRO_avg_time; \
} while (0);

#endif
