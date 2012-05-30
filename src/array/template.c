#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

double get_time(void)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

int main(int argc, char** argv)
{
    int element_count = atoi(argv[1]);
    int i, value = 0;

    if (argc <= 2)
    {
        return 1;
    }

    SETUP

    double before = get_time();
    if (!strcmp(argv[2], "insert"))
    {
        for (i = 0; i < element_count; i++)
        {
            INSERT(i, value);
        }
    }
    else if (!strcmp(argv[2], "delete"))
    {
        for (i = 0; i < element_count; i++)
        {
            INSERT(i, value);
        }

        before = get_time();
        for (i = 0; i < element_count; i++)
        {
            DELETE(i);
        }
    }
    else if (!strcmp(argv[2], "lookup"))
    {
        for (i = 0; i < element_count; i++)
        {
            INSERT(i, value);
        }
        before = get_time();
        for (i = 0; i < element_count; i++)
        {
            LOOKUP(i);
        }
    }
    else if (!strcmp(argv[2], "random-insert"))
    {
        srandom(1); // for a fair/deterministic comparison
        for (i = 0; i < element_count; i++)
        {
            int k = (int) random();
            INSERT(k % element_count, value);
        }
    }
    else if (!strcmp(argv[2], "random-delete"))
    {
        srandom(1); // for a fair/deterministic comparison
        for (i = 0; i < element_count; i++)
        {
            int k = (int) random();
            INSERT(k % element_count, value);
        }

        srandom(1); // for a fair/deterministic comparison
        for (i = 0; i < element_count; i++)
        {
            int k = (int) random();
            DELETE(k % element_count);
        }
    }
    else if (!strcmp(argv[2], "random-lookup"))
    {
        srandom(1); // for a fair/deterministic comparison
        for (i = 0; i < element_count; i++)
        {
            int k = (int) random();
            INSERT(k % element_count, value);
        }
        before = get_time();
        for (i = 0; i < element_count; i++)
        {
            int k = (int) random();
            LOOKUP(k % element_count);
        }
    }
    else if (!strcmp(argv[2], "random-usage"))
    {
        srandom(1); // for a fair/deterministic comparison
        for (i = 0; i < element_count / 2; i++)
        {
            int k = (int) random();
            INSERT(k % element_count, value);
        }

        before = get_time();
        for (i = 0; i < element_count; i++)
        {
            int a = (int) random();
            int k = (int) random();
            if (!(a++ % 3))
            {
                INSERT(k % element_count, value);
            }
            else if (!(a++ % 3))
            {
                DELETE(k % element_count);
            }
            else if (!(a++ % 3))
            {
                LOOKUP(k % element_count);
            }
        }
    }

    double after = get_time();
    printf("%f\n", after - before);
    fflush(stdout);
    sleep(1000000);
} /* main */
