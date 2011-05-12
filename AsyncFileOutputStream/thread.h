/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once

typedef void* thread_t;
typedef void (*thread_proc_t)(void *param);

thread_t thread_create(thread_proc_t start, void *param);
int thread_join(thread_t thread);
void thread_destroy(thread_t thread);
