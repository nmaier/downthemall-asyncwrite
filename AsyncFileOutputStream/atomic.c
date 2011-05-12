/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <pratom.h>
#include "atomic.h"

long atomic_increment(atomic_t *value) {
  return PR_AtomicIncrement((PRInt32*)value);
}
long atomic_decrement(atomic_t *value) {
  return PR_AtomicDecrement((PRInt32*)value);
}
long atomic_set(atomic_t *value, atomic_t newvalue) {
  return PR_AtomicSet((PRInt32*)value, (PRInt32)newvalue);
}
