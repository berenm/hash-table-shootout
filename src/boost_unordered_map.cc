#include <inttypes.h>
#include <boost/unordered_map.hpp>

typedef boost::unordered_map< int64_t, int64_t > hash_t;

#define SETUP hash_t hash;
#define INSERT_INT_INTO_HASH(key, value) hash.insert(hash_t::value_type(key, value))
#define LOOKUP_INT_FROM_HASH(key)        (void) hash.find(key)
#define DELETE_INT_FROM_HASH(key)        hash.erase(key)

#include "template.c"
