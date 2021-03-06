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
#include <stdio.h>
#endif

#include <pratom.h>
#include <prmon.h>
#include <prio.h>
#include <private/pprio.h>
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

monitor_t monitor_create() {
  return (monitor_t)PR_NewMonitor();
}
void monitor_enter(monitor_t monitor) {
  PR_EnterMonitor((PRMonitor*)monitor);
}
int monitor_join(monitor_t monitor) {
  PRStatus rv;
  rv = PR_Wait((PRMonitor*)monitor, PR_INTERVAL_NO_TIMEOUT);
  return rv == PR_SUCCESS ? 1 : 0;
}
void monitor_set(monitor_t monitor) {
  PR_Notify((PRMonitor*)monitor);
}
void monitor_leave(monitor_t monitor) {
  PR_ExitMonitor((PRMonitor*)monitor);
}
void monitor_destroy(monitor_t monitor) {
  PR_DestroyMonitor((PRMonitor*)monitor);
}

int file_seek(file_t file, PRInt64 offset) {
  return PR_Seek64((PRFileDesc*)file, offset, PR_SEEK_SET) == offset;
}

int file_write(file_t file, void *buffer, int amount) {
  int rv = PR_Write((PRFileDesc*)file, buffer, amount) == amount;
  file_flush(file);
  return rv;
}

void file_seteof(file_t file) {
#if defined(XP_WIN)
  SetEndOfFile((HANDLE)PR_FileDesc2NativeHandle((PRFileDesc*)file));

#elif defined(XP_UNIX)
  PRInt64 offset = PR_Seek64((PRFileDesc*)file, 0, SEEK_CUR);

  if (offset < 1) {
    return;
  }
  ftruncate(PR_FileDesc2NativeHandle((PRFileDesc*)file), offset);

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
