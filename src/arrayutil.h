#pragma once
#include <stdio.h>
#include <stdlib.h>

#define ArrayUtil_free(array) \
    if (array.items) { \
    array.len = 0; \
    free(array.items); \
    array.items = NULL; \
    }


#define ArrayUtil_create(array) \
    array.len = 0; \
    array.items = malloc(sizeof(*array.items))

#define ArrayUtil_add(array, item) \
    array.len++; \
    array.items = realloc(array.items, sizeof(*array.items) * array.len); \
    array.items[array.len] = item
