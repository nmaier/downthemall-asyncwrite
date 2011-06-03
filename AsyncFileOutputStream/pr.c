/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <mozilla-config.h>

#if defined(XP_WIN)
#include <windows.h>
#elif defined(XP_UNIX)
#include <unistd.h>
#include <sys/types.h>
#endif

#include <pratom.h>
#include <prmon.h>
#include <prio.h>
#include <private/pprio.h>
#include <prrwlock.h>
#include <prthread.h>

#include <plarena.h>

#include "pr.h"

long atomic_increment(atomic_t *value) {
  return PR_AtomicIncrement((PRInt32*)value);
}
long atomic_decrement(atomic_t *value) {
  return PR_AtomicDecrement((PRInt32*)value);
}
long atomic_set(atomic_t *value, atomic_t newvalue) {
  return PR_AtomicSet((PRInt32*)value, (PRInt32)newvalue);
}

event_t event_create() {
  return (event_t)PR_NewMonitor();
}
void event_enter(event_t event) {
  PR_EnterMonitor((PRMonitor*)event);
}
int event_join(event_t event) {
  PRStatus rv;
  rv = PR_Wait((PRMonitor*)event, PR_INTERVAL_NO_TIMEOUT);
  return rv == PR_SUCCESS ? 1 : 0;
}

void event_set(event_t event) {
  PR_Notify((PRMonitor*)event);
}
void event_leave(event_t event) {
  PR_ExitMonitor((PRMonitor*)event);
}

void event_destroy(event_t event) {
  PR_DestroyMonitor((PRMonitor*)event);
}

int file_seek(file_t file, __int64 offset) {
  return PR_Seek64((PRFileDesc*)file, offset, PR_SEEK_SET) == offset;
}

int file_write(file_t file, void *buffer, int amount) {
  return PR_Write((PRFileDesc*)file, buffer, amount) == amount;
}

void file_seteof(file_t file) {
#if defined(XP_WIN)
  SetEndOfFile((HANDLE)PR_FileDesc2NativeHandle((PRFileDesc*)file));

#elif defined(XP_UNIX) && defined(HAVE_TRUNCATE64)
  FILE *osfd = (FILE*)PR_FileDesc2NativeHandle((PRFileDesc*)file);
  off64_t pos;

  if (!osfd) {
    return;
  }
  pos = ftello64(osfd);
  if (pos < 0) {
    return;
  }
  ftruncate64(osfd, pos);

#elif defined(XP_UNIX)
  FILE *osfd = (FILE*)PR_FileDesc2NativeHandle((PRFileDesc*)file);
  off_t pos;

  if (!osfd) {
    return;
  }
  pos = ftello(osfd);
  if (pos < 0) {
    return;
  }
  ftruncate(osfd, pos);

#else
#error not implemented
#endif
}

void file_flush(file_t file) {
  PR_Sync((PRFileDesc*)file);
}

void file_close(file_t file) {
  PR_Close((PRFileDesc*)file);
}

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

pool_t* pool_create(size_t initial_size) {
  PLArenaPool *pool = (PLArenaPool*)malloc(sizeof(PLArenaPool));
  PL_InitArenaPool(pool, "dta pool", initial_size, 4096);
  return pool;
}
void *pool_alloc(pool_t *pool, size_t size) {
  return PL_ArenaAllocate((PLArenaPool*)pool, size);
}
void pool_free(pool_t *pool, void *ptr) {
  if (ptr) {
    PL_ArenaRelease((PLArenaPool*)pool, ptr);
  }
}
void pool_destroy(pool_t *pool) {
  PL_FinishArenaPool(pool);
  free(pool);
}

thread_t thread_create(thread_proc_t start, void *param) {
  thread_t *rv;
  return (thread_t)PR_CreateThread(
    PR_USER_THREAD,
    start,
    param,
    PR_PRIORITY_LOW,
    PR_GLOBAL_THREAD,
    PR_JOINABLE_THREAD,
    0
    );
  return rv;
}

int thread_join(thread_t thread) {
  if (!thread) {
    return 0;
  }
  return PR_JoinThread((PRThread*)thread) == PR_SUCCESS;
}

void thread_destroy(thread_t thread) {
  /* no op */
}
