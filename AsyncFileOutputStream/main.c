/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <windows.h>
#include "delayed_stream.h"

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
  if (dwReason == DLL_PROCESS_ATTACH) {
    delayed_stream_library_init();
  }
  else if (dwReason == DLL_PROCESS_DETACH) {
    delayed_stream_library_finish();
  }
  return TRUE;
}
