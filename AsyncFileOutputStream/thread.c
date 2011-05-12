/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <prthread.h>
#include "thread.h"

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
