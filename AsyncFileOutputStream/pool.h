/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once

#include <stddef.h>

typedef void pool_t;

pool_t* pool_create(size_t initial_size);
void *pool_alloc(pool_t *pool, size_t size);
void pool_free(pool_t *pool, void *ptr);
void pool_destroy(pool_t *pool);
