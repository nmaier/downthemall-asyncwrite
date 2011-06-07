/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once
#ifndef pr_h_
#define pr_h_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef long volatile atomic_t;

long atomic_increment(atomic_t *value);
long atomic_decrement(atomic_t *value);
long atomic_set(atomic_t *value, atomic_t newvalue);


typedef void* monitor_t;

monitor_t monitor_create();
void monitor_enter(monitor_t monitor);
int monitor_join(monitor_t monitor);
void monitor_set(monitor_t monitor);
void monitor_leave(monitor_t monitor);
void monitor_destroy(monitor_t monitor);


typedef void* file_t /* PRFileDesc* */;

int file_seek(file_t file, PRInt64 offset);
int file_write(file_t file, void *buffer, int amount);
void file_seteof(file_t file);
void file_flush(file_t file);
void file_close(file_t file);


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


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* pr_h_ */
