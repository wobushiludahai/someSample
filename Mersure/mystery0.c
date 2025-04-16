#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>

static const int kIterations = 1000 * 1000000;

int main(int argc, const char **argv)
{
    uint64_t startcy, stopcy;
    uint64_t sum = 0;

    startcy = __rdtsc();

    for (int i = 0; i < kIterations; ++i)
    {
        sum += i;
    }

    stopcy = __rdtsc();

    int64_t elapsed = stopcy - startcy;
    double felapsed = elapsed;
    fprintf(
        stdout, "%d iterations, %lu cycles, %4.2f cycles/iteration\n", kIterations, elapsed, felapsed / kIterations);

    return 0;
}