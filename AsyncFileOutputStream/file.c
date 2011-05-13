/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

#include <private/pprio.h>

#include <prio.h>
#include "file.h"


int file_seek(file_t file, __int64 offset) {
  return PR_Seek64((PRFileDesc*)file, offset, PR_SEEK_SET) == offset;
}

int file_write(file_t file, void *buffer, int amount) {
  return PR_Write((PRFileDesc*)file, buffer, amount) == amount;
}

void file_seteof(file_t file) {
#if defined(WIN32) || defined(_WIN32)
  SetEndOfFile((HANDLE) PR_FileDesc2NativeHandle((PRFileDesc*)file));
#else
  // XXX
#endif
}

void file_flush(file_t file) {
  PR_Sync((PRFileDesc*)file);
}

void file_close(file_t file) {
  PR_Close((PRFileDesc*)file);
}
