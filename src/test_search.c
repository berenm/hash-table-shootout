#include "test_search.h"

#include "test_search_dichotomic.h"
#include "test_search_dichotomic_bitwise.h"
#include "test_search_dichotomic_addshift.h"
#include "test_search_linear.h"

static int intcmp(void const* a, void const* b)
{
    return (*(uint64_t*) a > *(uint64_t*) b) ? 1 : (*(uint64_t*) a < *(uint64_t*) b ? -1 : 0);
}

typedef int (generator_t)(int k);

static int generate_zero(int k)
{
    return 0;
}

static int generate_random(int k)
{
    return random();
}

static int generate_sequence(int k)
{
    return k;
}

static int generate_sequence_value(int k)
{
    return k * ARRAY_SIZE + k % ARRAY_SIZE;
}

uint64_t get_nanoseconds()
{
    struct timespec time;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);

    return time.tv_sec * 1000000000 + time.tv_nsec;
}

void do_search(size_t const    array_count,
               size_t const    retry_count,
               array_t const*  arrays,
               uint64_t const* values,
               size_t*         results,
               search_t*       search,
               char const*     search_name)
{
    __sync_synchronize();
    uint64_t from = get_nanoseconds();
    for (int k = 0; k < retry_count; ++k)
        for (int i = 0; i < array_count; ++i)
            results[i] = search(arrays[i].size, arrays[i].values, values[i]);

    __sync_synchronize();
    uint64_t to          = get_nanoseconds();
    double   nanoseconds = to - from;
    printf("%s took total: %lfns, per: %lfns\n",
           search_name,
           nanoseconds,
           nanoseconds / (ARRAY_SIZE) / array_count / retry_count);
}

int main(int argc, char const* argv[])
{
    size_t array_count   = 100000;
    size_t element_count = ARRAY_SIZE;
    size_t retry_count   = 10;
    size_t printed_array = 0;

    srandom(1);

    if (argc > 1)
        array_count = atoi(argv[1]);

    if (argc > 2)
        retry_count = atoi(argv[2]);

    generator_t* generator       = generate_zero;
    generator_t* value_generator = generate_zero;

    if (argc > 3)
    {
        if (!strcmp(argv[3], "random"))
        {
            generator       = &generate_random;
            value_generator = &generate_random;
        }
        if (!strcmp(argv[3], "sequence"))
        {
            generator       = &generate_sequence;
            value_generator = &generate_sequence_value;
        }
    }

    if (argc > 4)
        printed_array = atoi(argv[4]);

    array_t*  arrays  = malloc(array_count * sizeof(array_t));
    uint64_t* values  = malloc(array_count * sizeof(uint64_t));
    size_t*   results = malloc(array_count * sizeof(size_t));

    printf(".. filling %zu arrays, with %zu elements\n", array_count, element_count);
    for (int i = 0; i < array_count; ++i)
    {
        for (int j = 0; j < element_count; ++j)
        {
            arrays[i].values[j] = generator(i * element_count + j);
            arrays[i].size++;
        }
        qsort(arrays[i].values, element_count, sizeof(uint64_t), &intcmp);
    }

    for (int i = 0; i < array_count; ++i)
        values[i] = value_generator(i);

    printf("arrays[%lu] size: %zu ", printed_array, arrays[printed_array].size);
    for (int i = 0; i < element_count; ++i)
        printf("%10lu ", arrays[printed_array].values[i]);

    printf("\n");
    printf("searching value %lu\n", values[printed_array]);

    do_search(array_count, retry_count, arrays, values, results, &linear_search, "linear search");
    printf("found %zu\n", results[printed_array]);
    do_search(array_count, retry_count, arrays, values, results, &dichotomic_search, "dichotomic search");
    printf("found %zu\n", results[printed_array]);
    do_search(array_count, retry_count, arrays, values, results, &dichotomic_search_bitwise, "dichotomic search bitwise");
    printf("found %zu\n", results[printed_array]);

    // do_search(array_count, retry_count, arrays, values, results, &dichotomic_search_addshift, "dichotomic search shift");
    // printf("found %zu\n", results[printed_array]);

    printf(".. freeing data\n");
    free(values);
    free(arrays);
    free(results);

    return 0;
} /* main */
