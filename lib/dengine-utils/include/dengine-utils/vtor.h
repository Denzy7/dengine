/*
 * Simple C++ vector like implementation
 *
 * #define VTOR_IMPLEMENTATION before
 * including this file
 *
 * If you get multiple definition errors
 * ensure you have defined VTOR_IMPLEMENTATION
 * only once in the code base
 *
 * Prefferably, define it in its own seperate file
 *
 * Optionally define VTOR_VERBOSE
 * before VTOR_IMPLEMENTATION to print
 * memory allocations
 */

#ifndef VTOR_H
#define VTOR_H

#include <stddef.h> //size_t

typedef struct
{
    void* data;
    size_t count;
    size_t capacity;
    size_t type_sz;
} vtor;

int vtor_create_alloc(vtor* vtor, size_t type_sz, size_t num);

int vtor_create(vtor* vtor, size_t type_sz);

void vtor_pushback(vtor* vtor, const void* val);

void vtor_popback(vtor* vtor);

void vtor_free(vtor* vtor);
#endif // VTOR_H
#ifdef VTOR_IMPLEMENTATION

#include <stdlib.h>  //malloc, free
#include <string.h>  //memcpy
#ifdef VTOR_VERBOSE
#include <stdio.h>
#endif

int vtor_create_alloc(vtor* vtor, size_t type_sz, size_t num)
{
    vtor->data = malloc(num * type_sz);
    vtor->capacity = num;
    vtor->type_sz = type_sz;
    vtor->count = 0;

#ifdef VTOR_VERBOSE
    if(vtor->data)
    {
        printf("create vec type_sz : %zu, capacity : %zu\n", vtor->type_sz, vtor->capacity);
    }else
    {
        printf("error cannot create vec. null vtor->data\n");
    }

#endif // VTOR_VERBOSE

    if(vtor->data)
        return 1;
    else
        return 0;
}

int vtor_create(vtor* vtor, size_t type_sz)
{
    return vtor_create_alloc(vtor, type_sz, 2);
}

void vtor_pushback(vtor* vtor, const void* val)
{
    size_t capacity = vtor->capacity;
    size_t type_sz = vtor->type_sz;

    if(vtor->count == vtor->capacity && type_sz > 0)
    {
        /* expand vtor */

        //temp buffer
        void* tmp = malloc(capacity * type_sz);
        memcpy(tmp, vtor->data, capacity * type_sz);

        //free old
        free(vtor->data);

        //realloc
        vtor->data = malloc(type_sz * capacity * 2);

        //copy back
        memcpy(vtor->data, tmp, type_sz * capacity);

        //free temp
        free(tmp);

        //set capacity
        vtor->capacity = capacity * 2;

#ifdef VTOR_VERBOSE
        printf("realloc vec type_sz : %zu, capacity : %zu\n", vtor->type_sz, vtor->capacity);
#endif // VTOR_VERBOSE
    }

    if(vtor->data)
    {
        void* new_loc = vtor->data + (vtor->type_sz * vtor->count);
        memcpy(new_loc, val, type_sz);
        vtor->count++;

#ifdef VTOR_VERBOSE
        printf("pushback vec type_sz : %zu, capacity : %zu, count : %zu\n", vtor->type_sz, vtor->capacity, vtor->count);
    }else{
        printf("cannot pushback. null vtor->data\n");
#endif // VTOR_VERBOSE

    }
}

void vtor_free(vtor* vtor)
{
    if(vtor->data)
    {
        free(vtor->data);

        vtor->data = NULL;

#ifdef VTOR_VERBOSE
        printf("free vec type_sz : %zu, capacity : %zu, count : %zu\n", vtor->type_sz, vtor->capacity, vtor->count);
    }else{
        printf("cannot free. null vtor->data\n");
#endif // VTOR_VERBOSE
    }
}

void vtor_popback(vtor* vtor)
{
    if(vtor->data && vtor->count > 0)
    {
        vtor->count--;
#ifdef VTOR_VERBOSE
        printf("popback vec type_sz : %zu, capacity : %zu, count : %zu\n", vtor->type_sz, vtor->capacity, vtor->count);
    }else{
        printf("cannot popback. null vtor->data\n");
#endif // VTOR_VERBOSE

        size_t capacity = vtor->capacity;
        size_t type_sz = vtor->type_sz;

        if(vtor->count == capacity / 2 && type_sz)
        {
            /* contract vtor */

            //temp buffer
            void* tmp = malloc((capacity / 2) * type_sz);
            memcpy(tmp, vtor->data, (capacity / 2) * type_sz);

            //free old
            free(vtor->data);

            //set temp
            vtor->data = tmp;

            //set capacity
            vtor->capacity = capacity / 2;

#ifdef VTOR_VERBOSE
            printf("contract vec type_sz : %zu, capacity : %zu, count : %zu\n", vtor->type_sz, vtor->capacity, vtor->count);
#endif
        }
    }
}

#endif

