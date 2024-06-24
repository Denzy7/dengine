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

typedef struct vtor
{
    void* data;
    size_t count;
    size_t capacity;
    size_t type_sz;
    char ptrs; /*!<are we storing pointers? */
} vtor;

int vtor_create_alloc(vtor* vtor, size_t type_sz, size_t num);

int vtor_create(vtor* vtor, size_t type_sz);

/* a vector holding pointers */
int vtor_create_ptrs(vtor* vtor);

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
int vtor_create_ptrs(vtor* vtor)
{
    vtor->ptrs = 1;
    return vtor_create_alloc(vtor, sizeof(void*), 2);
}

void _vtor_copytonewdata(vtor* vtor, void* newdata)
{
    if(vtor->ptrs)
    {
        char** oldref = vtor->data;
        char** newref = newdata;
        for(size_t i = 0; i < vtor->count; i++)
        {
            newref[i] = oldref[i]; 
        }
    }else {
        memcpy(newdata, vtor->data, vtor->type_sz * vtor->capacity);
    }
}
void vtor_pushback(vtor* vtor, const void* val)
{
    if(vtor->count == vtor->capacity)
    {
        /* expand vtor */
        size_t sz = vtor->type_sz * vtor->capacity * 2;
        void* newdata = malloc(sz);
        _vtor_copytonewdata(vtor, newdata);
        
        vtor->capacity *= 2;
        free(vtor->data);
        vtor->data = newdata;

#ifdef VTOR_VERBOSE
        printf("realloc vec type_sz : %zu, capacity : %zu\n", vtor->type_sz, vtor->capacity);
#endif // VTOR_VERBOSE
    }


    if(vtor->data)
    {
        void* new_loc = vtor->data + (vtor->type_sz * vtor->count);
        if(vtor->ptrs)
        {
            char** ref = vtor->data;
            ref[vtor->count] = (char*)val;
        }else {
            memcpy(new_loc, val, vtor->type_sz);
        }
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
#ifdef VTOR_VERBOSE
        printf("free vec type_sz : %zu, capacity : %zu, count : %zu\n", vtor->type_sz, vtor->capacity, vtor->count);
    }else{
        printf("cannot free. null vtor->data\n");
#endif // VTOR_VERBOSE
    }
}

void vtor_popback(vtor* vtor)
{
    vtor->count--;
#ifdef VTOR_VERBOSE
        printf("popback vec type_sz : %zu, capacity : %zu, count : %zu\n", vtor->type_sz, vtor->capacity, vtor->count);
#endif // VTOR_VERBOSE
    if(vtor->count == vtor->capacity / 2)
    {
        /* contract vtor */
        void* newdata = malloc((vtor->capacity / 2) * vtor->type_sz);
        _vtor_copytonewdata(vtor, newdata);
        free(vtor->data);
        vtor->data = newdata;

        //set capacity
        vtor->capacity /= 2;

#ifdef VTOR_VERBOSE
        printf("contract vec type_sz : %zu, capacity : %zu, count : %zu\n", vtor->type_sz, vtor->capacity, vtor->count);
#endif
    }
}

#endif

