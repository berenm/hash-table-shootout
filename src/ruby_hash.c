#include <ruby.h>

#define SETUP                           \
    ruby_init();                        \
    VALUE hash         = rb_hash_new(); \
    VALUE rb_int_value = INT2NUM(0);
#define INSERT_INT_INTO_HASH(key, value)              \
    do                                                \
    {                                                 \
        VALUE rb_int_key = INT2NUM(key); /* leak */   \
        rb_hash_aset(hash, rb_int_key, rb_int_value); \
    }                                                 \
    while (0)
#define LOOKUP_INT_FROM_HASH(key)                   \
    do                                              \
    {                                               \
        VALUE rb_int_key = INT2NUM(key); /* leak */ \
        (void) rb_hash_lookup(hash, rb_int_key);    \
    }                                               \
    while (0)
#define DELETE_INT_FROM_HASH(key)                   \
    do                                              \
    {                                               \
        VALUE rb_int_key = INT2NUM(key); /* leak */ \
        rb_hash_delete(hash, rb_int_key);           \
    }                                               \
    while (0)

#include "template.c"
