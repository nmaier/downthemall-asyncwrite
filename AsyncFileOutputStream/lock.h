/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once

typedef void * lock_t;

lock_t lock_create();
void lock_aquire_read(lock_t lock);
void lock_aquire_write(lock_t lock);
void lock_release(lock_t lock);
void lock_destroy(lock_t lock);
