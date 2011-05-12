/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <prmon.h>
#include "event.h"

event_t event_create() {
  return (event_t)PR_NewMonitor();
}

int event_join(event_t event) {
  PRStatus rv;
  PR_EnterMonitor((PRMonitor*)event);
  rv = PR_Wait((PRMonitor*)event, PR_INTERVAL_NO_TIMEOUT);
  PR_ExitMonitor((PRMonitor*)event);
  return rv == PR_SUCCESS ? 1 : 0;
}

void event_set(event_t event) {
  PR_EnterMonitor((PRMonitor*)event);
  PR_Notify((PRMonitor*)event);
  PR_ExitMonitor((PRMonitor*)event);
}

void event_destroy(event_t event) {
  PR_DestroyMonitor((PRMonitor*)event);
}
