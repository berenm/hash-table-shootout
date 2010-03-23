#include <Python.h>
typedef PyObject * hash_t;
#define SETUP \
    Py_Initialize(); \
    hash_t hash = PyDict_New(); \
    PyObject * py_int_value = PyInt_FromLong(0);
#define INSERT_INTO_HASH(key, value) do { \
        PyObject * py_int_key = PyInt_FromLong(key); /* leak */ \
        PyDict_SetItem(hash, py_int_key, py_int_value); \
    } while(0)
#include "template.c"