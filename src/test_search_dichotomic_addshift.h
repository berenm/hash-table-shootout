#include "test_search.h"

uint8_t dichotomic_search_addshift(uint8_t const key_count, uint64_t const* const keys, uint64_t key)
{
    /* Few obvious cases that need to be resolved so we can start iterating at the middle */
    if (keys[0] > key)
        return 0;

    if (keys[key_count - 1] <= key)
        return key_count - 1;


    /* Jump by power of two steps, smaller and smaller, starting at the middle of the array.
     *
     * We might jump on the actual result without knowing it until we've checked the lower keys
     * so we might have to correct the result at the end.
     */
    uint8_t index = ARRAY_SIZE >> 1;
    uint8_t step  = ARRAY_SIZE >> 2;
    do
    {
        if (keys[index] > key)
            index -= step;

        else
            index += step;

        step >>= 1;
    }
    while (step > 0);

    /* Correct if we've missed the exact position */
    if (keys[index] > key)
        index--;

    return index;
}
