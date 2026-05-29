#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

/**
 * Macro for generically typed dynamic arrays
 * Parameters
 * - type: the type that each object in the array has
 * e.g. if type == int, then this is an integer array
 * - label: the name given to the struct; basically the type name for the struct
 * 
 * 
 * _append function allows a new element of type _type_ to be added to the end of 
 * the dynamic array. Handles reallocation if capacity is full.
 * 
 * _init function creates a new instance of the struct, initialising all fields to
 * 'default' values; empty malloced array, size == 0 and a capacity set to the 
 * capacity given as a parameter.
 */
#define DECLARE_DYNAMIC_ARRAY(type, label) \
    typedef struct { \
        type* data; \
        int size; \
        int capacity; \
    } label; \
    \
    static inline void label##_append(label* arr, type value) \
    { \
        if (arr->size >= arr->capacity) \
        { \
            arr->capacity = arr->capacity > 0 ? arr->capacity * 2 : 1; \
            arr->data = realloc(arr->data, sizeof(type) * arr->capacity); \
        } \
        if (!(arr->data)) \
        { \
            printf("In dynamic_array.h._append: Realloc seems to have failed -- may cause data issues!\n"); \
        } \
        arr->data[(arr->size)++] = value; \
    } \
    \
    static inline label* label##_init(int capacity) \
    { \
        label* template = malloc( sizeof(label) ); \
        if (!template) \
        { \
            fprintf(stderr, "In dynamic_array.h._init: Failed to malloc the template\n"); \
        } \
        template->size = 0; \
        template->capacity = capacity;\
        template->data = malloc(capacity * sizeof(type)); \
        if (!(template->data)) \
        { \
            fprintf(stderr, "In dynamic_array.h._init: Failed to malloc the template->data\n"); \
        } \
        return template; \
    } \
    \
    static inline void label##_free(label* arr) \
    { \
        free(arr->data); \
        free(arr); \
    } \
    \
    static inline void label##_pop(label* arr) \
    {  \
        (arr->size)--; \
    } \


# endif // DYNAMIC_ARRAY_H