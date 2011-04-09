/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include "lock.h"
#include "prrwlock.h"

lock_t lock_create() {
  return PR_NewRWLock(PR_RWLOCK_RANK_NONE, "dta lock");
}
void lock_aquire_read(lock_t lock) {
  PR_RWLock_Rlock((PRRWLock*)lock);
}
void lock_aquire_write(lock_t lock) {
  PR_RWLock_Wlock((PRRWLock*)lock);
}
void lock_release(lock_t lock) {
  PR_RWLock_Unlock((PRRWLock*)lock);
}
void lock_destroy(lock_t lock) {
  PR_DestroyRWLock((PRRWLock*)lock);
}
