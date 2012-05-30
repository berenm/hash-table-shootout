#include <stdint.h>
#include <stdlib.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

static uint64_t get_nanoseconds()
{
    struct timespec timestamp;
    clock_gettime(CLOCK_REALTIME, &timestamp);

    uint64_t const seconds_since_midnight     = (uint64_t) (timestamp.tv_sec % (3600 * 24));
    uint64_t const nanoseconds_since_midnight = seconds_since_midnight * (uint64_t) 1000000000 + (uint64_t) timestamp.tv_nsec;

    return nanoseconds_since_midnight;
}

#ifndef ARRAY_SIZE
# define ARRAY_SIZE 16
#endif /* ifndef ARRAY_SIZE */

typedef struct
{
    size_t   size;
    uint64_t values[ARRAY_SIZE];
} array_t;

static int intcmp(void const* a, void const* b)
{
    return (*(uint64_t*) a < *(uint64_t*) b) ? -1 : ((*(uint64_t*) a > *(uint64_t*) b) ? 1 : 0);
}

typedef uint64_t (generator_t)(int k);

static void fill_array(array_t* array_m, size_t const elements_count, generator_t* generator_m)
{
    for (uint64_t k = 0; k < ARRAY_SIZE && k < elements_count; ++k)
    {
        array_m->values[k] = generator_m(k);
        array_m->size++;
    }
    qsort(array_m->values, array_m->size, sizeof(*array_m->values), &intcmp);
}

static array_t* alloc_arrays(size_t const array_count, size_t const elements_count, generator_t* generator_m)
{
    array_t* arrays = calloc(array_count, sizeof(array_t));
    printf("allocating %d arrays\n", array_count);
    for (uint64_t i = 0; i < array_count; ++i)
    {
        fill_array(arrays + i, elements_count, generator_m);
    }
    fprintf(stdout, "array contents: [%d] ", arrays[0].size);
    for (uint64_t k = 0; k < ARRAY_SIZE && k < elements_count; ++k)
    {
        fprintf(stdout, "%d ", arrays[0].values[k]);
    }
    fprintf(stdout, "\n");

    return arrays;
}

typedef size_t (search_t)(size_t const array_size, array_t const* array, uint64_t value);

#include <boost/preprocessor.hpp>

#define linsearch_unrolled(z, n, _)                    \
    if (n >= array->size || array->values[n] >= value) \
        return n;

static size_t linsearch_unroll(size_t const array_size, array_t const* array, uint64_t value)
{
    BOOST_PP_REPEAT(ARRAY_SIZE, linsearch_unrolled, ~)
    return array->size;
}

#define linsearch_unrolled2(z, n, _) \
    index += (array->values[n] < value && n < array->size);

static size_t linsearch_unroll2(size_t const array_size, array_t const* array, uint64_t value)
{
    size_t index;
    BOOST_PP_REPEAT(ARRAY_SIZE, linsearch_unrolled2, ~)
    return index;
}

static size_t linsearch(size_t const array_size, array_t const* array, uint64_t value)
{
    uint64_t const* const begin = array->values;
    uint64_t const* const end   = begin + array->size;
    uint64_t const*       item  = begin;
    while (item < end)
    {
        if (*(item++) >= value)
            break;
    }

    return item - begin;
}

static size_t forsearch(size_t const array_size, array_t const* array, uint64_t value)
{
    size_t i;
    for (i = 0; i < array->size && array->values[i] < value && i < ARRAY_SIZE; ++i)
    {}
    return i;
}

static size_t dichsearch(size_t const array_size, array_t const* array, uint64_t value)
{
#define middle(b, e, v) (b + (e - b) / 2)
    uint64_t const* begin = array->values;
    uint64_t const* end   = begin + array->size;
    uint64_t const* item  = middle(begin, end, value);
    while (begin < end)
    {
        if (*item >= value)
            end = item;
        else
            begin = item + 1;
        item = middle(begin, end, value);
    }
#undef middle

    return item - array->values;
}

static size_t dichsearch2(size_t const array_size, array_t const* array, uint64_t value)
{
#define middle(b, e, v) (b + ((e - b) * (v - *b)) / (*(e - 1) - *b))
    uint64_t const* begin = array->values;
    uint64_t const* end   = begin + array->size;
    uint64_t const* item  = middle(begin, end, value);

    while (begin < end)
    {
        if (value < *begin)
            break;
        if (value >= *(end - 1))
            break;

        item = middle(begin, end, value);
        if (*item >= value)
            end = item;
        else
            begin = item + 1;
    }
#undef middle

    return item - array->values;
}

static void search_arrays(array_t* arrays, size_t const array_count, size_t const retry_count, uint64_t* values, search_t* search, const char* search_name)
{
    uint64_t const before = get_nanoseconds();
    for (uint64_t j = 0; j < retry_count; ++j)
    {
        for (uint64_t i = 0; i < array_count; ++i)
        {
            search(ARRAY_SIZE, arrays + i, values[j]);
        }
    }
    uint64_t const after       = get_nanoseconds();
    double         nanoseconds = after - before;

    size_t index = search(ARRAY_SIZE, arrays, values[0]);
    fprintf(stdout, "%s for value: %d (found %zu), "
            "%zu arrays "
            "%zu retries, "
            "total: %lfns, "
            "each: %lfns\n",
            search_name,
            values[0],
            index,
            array_count,
            retry_count,
            nanoseconds,
            nanoseconds / array_count / retry_count / ARRAY_SIZE);
    fflush(stdout);
    sleep(1);
}

static uint64_t generate_zeros(int k)
{
    return 0;
}

static uint64_t generate_random(int k)
{
    return (uint64_t) random();
}

static uint64_t generate_sequence(int k)
{
    return k;
}

static uint64_t generate_sequence_step(int k)
{
    return k * (ARRAY_SIZE + 1);
}

int main(int argc, char const* argv[])
{
    if (argc < 4)
        return 1;

    size_t const array_count = atoi(argv[1]);
    size_t const retry_count = atoi(argv[2]);

    size_t       size            = 0;
    generator_t* generator       = &generate_zeros;
    generator_t* value_generator = &generate_zeros;

    if (!strcmp(argv[3], "empty"))
    {
        size            = 0;
        generator       = &generate_zeros;
        value_generator = &generate_zeros;
    }
    else if (!strcmp(argv[3], "full-random"))
    {
        srandom(1);
        size            = ARRAY_SIZE;
        generator       = &generate_random;
        value_generator = &generate_random;
    }
    else if (!strcmp(argv[3], "sequence"))
    {
        srandom(1);
        size            = ARRAY_SIZE;
        generator       = &generate_sequence;
        value_generator = &generate_sequence;
    }

    uint64_t* values = malloc(retry_count * sizeof(uint64_t));
    for (uint64_t j = 0; j < retry_count; ++j)
    {
        values[j] = value_generator(j);
    }

    array_t* arrays = alloc_arrays(array_count, size, generator);

if(argc < 5 || !strcmp(argv[4], "forsearch"))
    search_arrays(arrays, array_count, retry_count, values, &forsearch, "forsearch");

if(argc < 5 || !strcmp(argv[4], "forsearch"))
    search_arrays(arrays, array_count, retry_count, values, &linsearch_unroll, "linsearch_unroll");

if(argc < 5 || !strcmp(argv[4], "forsearch"))
    search_arrays(arrays, array_count, retry_count, values, &dichsearch, "dichsearch");

if(argc < 5 || !strcmp(argv[4], "forsearch"))
    search_arrays(arrays, array_count, retry_count, values, &linsearch, "linsearch");

    free(arrays);
    free(values);

    return 0;
}
