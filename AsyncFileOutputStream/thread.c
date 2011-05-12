/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <windows.h>
#include "thread.h"

typedef struct __thread_proc_t {
  thread_proc_t start;
  void *param;
} _thread_proc_t;

static DWORD WINAPI _thread_proc(LPVOID *param) {
  _thread_proc_t *proc = (_thread_proc_t*)param;
  proc->start(proc->param);
  free(param);
  return 0;
}

thread_t thread_create(thread_proc_t start, void *param) {
  thread_t *rv;
  _thread_proc_t *proc = (_thread_proc_t*)malloc(sizeof(_thread_proc_t));

  if (!proc) {
    return NULL;
  }

  proc->start = start;
  proc->param = param;

  rv = (thread_t)CreateThread(
    NULL,
    0,
    _thread_proc,
    (LPVOID*)proc,
    0,
    NULL
    );
  if (!rv) {
    free(proc);
  }

  return rv;
}

int thread_join(thread_t thread) {
  if (!thread) {
    return 0;
  }
  return WaitForSingleObject((HANDLE)thread, INFINITE) == WAIT_OBJECT_0;
}

void thread_destroy(thread_t thread) {
  if (!thread) {
    return;
  }
  CloseHandle((HANDLE)thread);
}
