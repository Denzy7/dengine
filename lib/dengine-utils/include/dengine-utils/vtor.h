#ifndef VTOR_H
#define VTOR_H

#include <stdlib.h>

#ifdef VTOR_VERBOSE
#include <stdio.h>
#endif

typedef struct vtor
{
    void* dat;
    size_t cnt;
    size_t cap;
    size_t type_sz;
} vtor;

void vtor_create_alloc(vtor* vtor, size_t type_sz, size_t num)
{
    vtor->dat = malloc(num * type_sz);
    vtor->cap = num;
    vtor->type_sz = type_sz;

#ifdef VTOR_VERBOSE
    printf("create_alloc vec type_sz : %zu, cap : %zu\n", vtor->type_sz, vtor->cap);
#endif // VTOR_VERBOSE
}

void vtor_create(vtor* vtor, size_t type_sz)
{
    vtor->dat = malloc(2 * type_sz);
    vtor->cap = 2;
    vtor->type_sz = type_sz;

#ifdef VTOR_VERBOSE
    if(vtor->dat)
    {
        printf("create vec type_sz : %zu, cap : %zu\n", vtor->type_sz, vtor->cap);
    }else
    {
        printf("error cannot create vec. null vtor->dat\n");
    }

#endif // VTOR_VERBOSE
}

void vtor_pushback(vtor* vtor, void* val)
{
    unsigned int cap = vtor->cap;
    size_t type_sz = vtor->type_sz;

    if(vtor->cnt == vtor->cap && type_sz > 0)
    {
        //temp buffer
        void* tmp = malloc(cap * type_sz);
        memcpy(tmp, vtor->dat, cap * type_sz);

        //free old
        free(vtor->dat);

        //realloc
        vtor->dat = malloc(type_sz * cap * 2);

        //copy back
        memcpy(vtor->dat, tmp, type_sz * cap);

        //free temp
        free(tmp);

        //set cap
        vtor->cap = cap * 2;

#ifdef VTOR_VERBOSE
        printf("realloc vec type_sz : %zu, cap : %zu\n", vtor->type_sz, vtor->cap);
#endif // VTOR_VERBOSE
    }

    if(vtor->dat)
    {
        void* new_loc = vtor->dat + (vtor->type_sz * vtor->cnt);
        memcpy(new_loc, val, type_sz);
        vtor->cnt++;

#ifdef VTOR_VERBOSE
        printf("pushback vec type_sz : %zu, cap : %zu, cnt : %zu\n", vtor->type_sz, vtor->cap, vtor->cnt);
    }else{
        printf("cannot pushback. null vtor->dat\n");
#endif // VTOR_VERBOSE

    }
}

void vtor_free(vtor* vtor)
{
    if(vtor->dat)
    {
        free(vtor->dat);

        vtor->dat = NULL;

#ifdef VTOR_VERBOSE
        printf("free vec type_sz : %zu, cap : %zu, cnt : %zu\n", vtor->type_sz, vtor->cap, vtor->cnt);
    }else{
        printf("cannot free. null vtor->dat\n");
#endif // VTOR_VERBOSE
    }
}

void vtor_popback(vtor* vtor)
{
    if(vtor->dat && vtor->cnt > 0)
    {

        vtor->cnt--;
#ifdef VTOR_VERBOSE
        printf("popback vec type_sz : %zu, cap : %zu, cnt : %zu\n", vtor->type_sz, vtor->cap, vtor->cnt);
    }else{
        printf("cannot popback. null vtor->dat\n");
#endif // VTOR_VERBOSE
    }
}

#endif // VTOR_H
