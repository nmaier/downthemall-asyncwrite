/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is DownThemAll! delayed_stream.
 *
 * The Initial Developer of the Original Code is
 * Nils Maier.
 * Portions created by the Initial Developer are Copyright (C) 2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Nils Maier <maierman@web.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <windows.h>

#include "pool.h"
#include "lock.h"

typedef struct _stream {
  pool_t *pool;
  HANDLE fd;
  HANDLE hEvent;
  long volatile pending;
  long volatile open;
} stream_t;

typedef enum _qi_type {
  QI_REGULAR,
  QI_EOF,
  QI_POISIONPILL
} qi_type_t;

typedef struct _queue_item {
  struct _queue_item *prev;
  struct _queue_item *next;

  qi_type_t type;
  __int64 offset;
  char *bytes;
  size_t length;
  stream_t *stream;
} queue_item_t;

static queue_item_t *queue_item_create_empty(qi_type_t type) {
  queue_item_t *rv = (queue_item_t *)malloc(sizeof(queue_item_t));
  if (!rv) {
    return rv;
  }

  rv->prev = rv->next = NULL;
  rv->bytes = NULL;
  rv->stream = NULL;
  rv->type = type;

  return rv;
}

static queue_item_t *queue_item_create(stream_t *stream, __int64 offset, const char *bytes, size_t length) {
  queue_item_t *rv = queue_item_create_empty(QI_REGULAR);
  if (!rv) {
    return rv;
  }

  rv->offset = offset;
  rv->bytes = (char*)pool_alloc(stream->pool, length);
  if (!rv->bytes) {
    free(rv);
    return NULL;
  }
  memcpy(rv->bytes, bytes, length);
  rv->length = length;
  rv->stream = stream;

  return rv;
}
static queue_item_t *queue_item_create_poisonpill() {
  return queue_item_create_empty(QI_POISIONPILL);
}
static queue_item_t *queue_item_create_eof(stream_t *stream, __int64 size_hint) {
  queue_item_t *rv = queue_item_create_empty(QI_EOF);
  if (!rv) {
    return rv;
  }
  rv->offset = size_hint;
  rv->stream = stream;

  return rv;
}
static void queue_item_destroy(queue_item_t *item) {
  if (!item) {
    return;
  }
  if (item->bytes) {
    pool_free(item->stream->pool, item->bytes);
  }
  if (item->stream) {
    InterlockedDecrement(&item->stream->pending);
    SetEvent(item->stream->hEvent);
  }
  free(item);
}

typedef struct _queue {
  queue_item_t *front;
  queue_item_t *back;
  size_t length;
  lock_t lock;
  HANDLE hWorkAvailable;
} queue_t;

static queue_t *queue_create() {
  queue_t *rv = (queue_t*)malloc(sizeof(queue_t));
  if (!rv) {
    return rv;
  }

  rv->length = 0;
  rv->front = rv->back = NULL;
  rv->lock = lock_create();
  if (!rv->lock) {
    abort();
  }
  rv->hWorkAvailable = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (rv->hWorkAvailable == NULL) {
    abort();
  }

  return rv;
}

static void queue_push(queue_t *queue, queue_item_t *item) {
  lock_aquire_write(queue->lock);

  if (!queue->back) {
    queue->back = queue->front = item;
    item->prev = item->next = NULL;
    queue->length = 1;
    goto out;
  }

  queue->back->next = item;
  item->prev = queue->back;
  item->next = NULL;
  queue->back = item;
  queue->length++;

out:
  if (item->stream) {
    InterlockedIncrement(&item->stream->pending);
  }
  SetEvent(queue->hWorkAvailable);
  lock_release(queue->lock);
}
static queue_item_t *queue_shift(queue_t *queue) {
  queue_item_t *rv = NULL;

  lock_aquire_read(queue->lock);
  if (!queue->front) {
    goto out;
  }
  lock_release(queue->lock);
  lock_aquire_write(queue->lock);
  rv = queue->front;
  --queue->length;
  if (!queue->length) {
    queue->front = queue->back = NULL;
  }
  else {
    rv->next->prev = NULL;
    queue->front = rv->next;
  }
  rv->prev = NULL;
  rv->next = NULL;

out:
  lock_release(queue->lock);
  return rv;
}

static void queue_destroy(queue_t *queue) {
  queue_item_t *i;

  while ((i = queue_shift(queue)) != NULL) {
    queue_item_destroy(i);
  }

  lock_destroy(queue->lock);
  CloseHandle(queue->hWorkAvailable);
  free(queue);
}

typedef struct _libary {
  HANDLE hThread;
  queue_t *queue;
} library_t;

library_t *glibrary;


static DWORD WINAPI library_threadproc(LPVOID param) {
  library_t *library = (library_t*)param;
  queue_item_t *item;
  LARGE_INTEGER offset;
  DWORD written;
  while (WaitForSingleObject(library->queue->hWorkAvailable, INFINITE) == WAIT_OBJECT_0) {
    for (item = queue_shift(library->queue); item; item = queue_shift(library->queue)) {
      if (item->type == QI_POISIONPILL) {
        queue_item_destroy(item);
        return 0;
      }
      offset.QuadPart = item->offset;
      if (!SetFilePointerEx(item->stream->fd, offset, NULL, FILE_BEGIN)) {
        InterlockedExchange(&item->stream->open, 0);
      }
      else if (item->type == QI_EOF) {
        SetEndOfFile(item->stream->fd);
      }
      else if (!WriteFile(item->stream->fd, item->bytes, item->length, &written, NULL)) {
        InterlockedExchange(&item->stream->open, 0);
      }
      queue_item_destroy(item);
    }
  }

  // make compiler happy
  return 0;
}

void delayed_stream_library_init() {
  glibrary = (library_t *)malloc(sizeof(library_t));
  glibrary->hThread = CreateThread(NULL, 0, library_threadproc, glibrary, 0, NULL);
  if (glibrary->hThread == NULL) {
    abort();
  }
  glibrary->queue = queue_create();
  if (!glibrary->queue) {
    abort();
  }
}
void delayed_stream_library_finish() {
  queue_push(glibrary->queue, queue_item_create_poisonpill());
  WaitForSingleObject(glibrary->hThread, INFINITE);
  queue_destroy(glibrary->queue);
  CloseHandle(glibrary->hThread);
}

void* delayed_stream_open(wchar_t *file, __int64 size_hint) {
  pool_t *pool;
  stream_t *stream;

  if (size_hint > 0) {
    pool = pool_create(size_hint > (1<<22) ? (1<<22) : (size_t)size_hint);
  }
  else {
    pool = pool_create((1<<21));
  }
  if (!pool) {
    return NULL;
  }

  stream = (stream_t*)malloc(sizeof(stream_t));
  stream->pool = pool;
  stream->open = 1;
  stream->pending = 0;
  stream->fd = CreateFile(
    file,
    GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE,
    NULL,
    OPEN_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    NULL
    );
  if (stream->fd == INVALID_HANDLE_VALUE) {
    goto error_cf;
  }
  stream->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (stream->hEvent == NULL) {
    goto error_he;
  }

  if (size_hint > 0) {
    queue_item_t *eof = queue_item_create_eof(stream, size_hint);
    if (eof) {
      queue_push(glibrary->queue, eof);
    }
  }

  return stream;

error_he:
  CloseHandle(stream->fd);

error_cf:
  free(stream);
  pool_destroy(pool);
  return NULL;
}

int delayed_stream_write(stream_t *stream, __int64 offset, const char *bytes, size_t length) {
  queue_item_t *item;

  if (!stream || !bytes || !length) {
    return 0;
  }
  if (!stream->open) {
    return 0;
  }

  item = queue_item_create(stream, offset, bytes, length);
  if (!item) {
    return 0;
  }
  queue_push(glibrary->queue, item);
  return 1;
}

void delayed_stream_flush(stream_t *stream) {
  while (stream->pending > 0 && WaitForSingleObject(stream->hEvent, INFINITE) == WAIT_OBJECT_0);
  FlushFileBuffers(stream->fd);
}

void delayed_stream_close(stream_t *stream) {
  if (!stream) {
    return;
  }

  // Close the stream
  InterlockedExchange(&stream->open, 0);

  // Wait for all work to finish
  delayed_stream_flush(stream);

  CloseHandle(stream->fd);
  pool_destroy(stream->pool);
  free(stream);
}
