/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <windows.h>
#include "event.h"

event_t event_create() {
  return (event_t)CreateEvent(NULL, FALSE, FALSE, NULL);
}

event_result_t event_wait(event_t event, unsigned int timeout) {
  DWORD wr;
  if (!event) {
    return EVENT_RESULT_ERROR;
  }
  wr = WaitForSingleObject((HANDLE)event, timeout == EVENT_WAIT_INFINITE ? INFINITE : timeout);
  switch (wr) {
  case WAIT_OBJECT_0:
    return EVENT_RESULT_SUCCESS;
  case WAIT_TIMEOUT:
    return EVENT_RESULT_TIMEOUT;
  default:
    return EVENT_RESULT_ERROR;
  }
}

void event_set(event_t event) {
  SetEvent((HANDLE)event);
}

void event_destroy(event_t event) {
  CloseHandle((HANDLE)event);
}
