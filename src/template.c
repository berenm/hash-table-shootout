#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#ifndef TEARDOWN
# define TEARDOWN
#endif /* ifndef TEARDOWN */

double get_time(void) {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

char* new_string_from_integer(int num) {
  int   ndigits = num == 0 ? 1 : (int) log10(num) + 1;
  char* str     = (char*) malloc(11);

  sprintf(str, "%d", num);
  return str;
}

int main(int argc, char** argv) {
  int num_keys = atoi(argv[1]);
  int i, value = 0;

  if (argc <= 2) {
    return 1;
  }

  SETUP

  double before = get_time();
  if (!strcmp(argv[2], "insert")) {
    for (i = 0; i < num_keys; i++) {
      INSERT_INT_INTO_HASH(i, value);
    }
  } else if (!strcmp(argv[2], "lookup")) {
    for (i = 0; i < num_keys; i++) {
      INSERT_INT_INTO_HASH(i, value);
    }
    before = get_time();
    for (i = 0; i < num_keys; i++) {
      LOOKUP_INT_FROM_HASH(i);
    }
  } else if (!strcmp(argv[2], "delete")) {
    for (i = 0; i < num_keys; i++) {
      INSERT_INT_INTO_HASH(i, value);
    }
    before = get_time();
    for (i = 0; i < num_keys; i++) {
      DELETE_INT_FROM_HASH(i);
    }
  } else if (!strcmp(argv[2], "random-insert")) {
    srandom(1); // for a fair/deterministic comparison
    for (i = 0; i < num_keys; i++) {
      int k = (int) random();
      INSERT_INT_INTO_HASH(k, value);
    }
  } else if (!strcmp(argv[2], "random-lookup")) {
    srandom(1); // for a fair/deterministic comparison
    for (i = 0; i < num_keys; i++) {
      int k = (int) random();
      INSERT_INT_INTO_HASH(k, value);
    }
    srandom(1); // for a fair/deterministic comparison
    before = get_time();
    for (i = 0; i < num_keys; i++) {
      (int) random();
      int k = (int) random();
      LOOKUP_INT_FROM_HASH(k);
    }
  } else if (!strcmp(argv[2], "random-delete")) {
    srandom(1); // for a fair/deterministic comparison
    for (i = 0; i < num_keys; i++) {
      int k = (int) random();
      INSERT_INT_INTO_HASH(k, value);
    }
    srandom(1); // for a fair/deterministic comparison
    before = get_time();
    for (i = 0; i < num_keys; i++) {
      (int) random();
      int k = (int) random();
      DELETE_INT_FROM_HASH(k);
    }
  } else if (!strcmp(argv[2], "random-usage")) {
    srandom(1); // for a fair/deterministic comparison
    for (i = 0; i < num_keys; i++) {
      int a = (int) random();
      int k = (int) random();
      if (!(a++ % 2)) {
        continue;
      } else if (!(a++ % 2)) {
        INSERT_INT_INTO_HASH(k, value);
      }
    }

    srandom(1);
    before = get_time();
    for (i = 0; i < num_keys; i++) {
      int a = (int) random();
      int k = (int) random();
      if (!(a++ % 2)) {
        INSERT_INT_INTO_HASH(k, value);
      } else if (!(a++ % 2)) {
        DELETE_INT_FROM_HASH(k);
      }
    }
  }

  double after = get_time();
  printf("%f\n", after - before);
  fflush(stdout);
  sleep(2);

  TEARDOWN
} /* main */
