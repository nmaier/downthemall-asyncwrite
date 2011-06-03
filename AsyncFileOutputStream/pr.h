/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once
#ifndef pr_h_
#define pr_h_

#include <stdlib.h>

typedef long volatile atomic_t;

long atomic_increment(atomic_t *value);
long atomic_decrement(atomic_t *value);
long atomic_set(atomic_t *value, atomic_t newvalue);


typedef void* event_t;

event_t event_create();
void event_enter(event_t event);
int event_join(event_t event);
void event_set(event_t event);
void event_leave(event_t event);
void event_destroy(event_t event);


typedef void* file_t /* PRFileDesc* */;

int file_seek(file_t file, __int64 offset);
int file_write(file_t file, void *buffer, int amount);
void file_seteof(file_t file);
void file_flush(file_t file);
void file_close(file_t file);


typedef void * lock_t;

lock_t lock_create();
void lock_aquire_read(lock_t lock);
void lock_aquire_write(lock_t lock);
void lock_release(lock_t lock);
void lock_destroy(lock_t lock);


typedef void pool_t;

pool_t* pool_create(size_t initial_size);
void *pool_alloc(pool_t *pool, size_t size);
void pool_free(pool_t *pool, void *ptr);
void pool_destroy(pool_t *pool);


typedef void* thread_t;
typedef void (*thread_proc_t)(void *param);

thread_t thread_create(thread_proc_t start, void *param);
int thread_join(thread_t thread);
void thread_destroy(thread_t thread);

#endif /* pr_h_ */
