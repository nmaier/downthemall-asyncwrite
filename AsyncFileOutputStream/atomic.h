/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once

typedef long volatile atomic_t;

long atomic_increment(atomic_t *value);
long atomic_decrement(atomic_t *value);
long atomic_set(atomic_t *value, atomic_t newvalue);
