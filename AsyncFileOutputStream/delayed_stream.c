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

#include <string.h>
#include <prtypes.h>

#include "pr.h"

typedef struct _stream {
  pool_t *pool;
  file_t fd;
  monitor_t monitor;
  atomic_t pending;
  atomic_t open;
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
  PRInt64 offset;
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

static queue_item_t *queue_item_create(stream_t *stream, PRInt64 offset, const char *bytes, size_t length) {
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
static queue_item_t *queue_item_create_eof(stream_t *stream, PRInt64 size_hint) {
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
  free(item);
}
static void queue_item_destroy_stream(queue_item_t *item) {
  monitor_t monitor = item->stream->monitor;
  monitor_enter(monitor);
  --item->stream->pending;
  queue_item_destroy(item);
  monitor_set(monitor);
  monitor_leave(monitor);
}

typedef struct _queue {
  queue_item_t *front;
  queue_item_t *back;
  size_t length;
  monitor_t workAvailable;
} queue_t;

static queue_t *queue_create() {
  queue_t *rv = (queue_t*)malloc(sizeof(queue_t));
  if (!rv) {
    return rv;
  }

  rv->length = 0;
  rv->front = rv->back = NULL;
  rv->workAvailable = monitor_create();
  if (rv->workAvailable == NULL) {
    abort();
  }

  return rv;
}

static void queue_push(queue_t *queue, queue_item_t *item) {
  monitor_enter(queue->workAvailable);
  if (item->stream) {
  monitor_enter(item->stream->monitor);
  }

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
  ++item->stream->pending;
  monitor_set(item->stream->monitor);
  monitor_leave(item->stream->monitor);
  }
  monitor_set(queue->workAvailable);
  monitor_leave(queue->workAvailable);
}
static queue_item_t *queue_shift(queue_t *queue) {
  queue_item_t *rv, *i;

  monitor_enter(queue->workAvailable);
  while (!queue->length) {
  monitor_join(queue->workAvailable);
  }

  /* if there is nothing better, then pop front */
  rv = queue->front;

  for (i = queue->front; i; i = i->next) {
    /* prefer closed streams */
    if (i->type == QI_REGULAR && i->stream && !i->stream->open) {
      rv = i;
      break;
    }
    /* prefer EOF */
    if (i->type == QI_EOF) {
      rv = i;
    }
  }

  --queue->length;
  if (!queue->length) {
    queue->front = queue->back = NULL;
  }
  else if (rv->next && rv->prev) {
    rv->next->prev = rv->prev;
    rv->prev->next = rv->next;
  }
  else if (rv->prev) {
    rv->prev->next = NULL;
    queue->back = rv->prev;
  }
  else {
    rv->next->prev = NULL;
    queue->front = rv->next;
  }
  rv->prev = NULL;
  rv->next = NULL;

  monitor_leave(queue->workAvailable);
  return rv;
}

static void queue_destroy(queue_t *queue) {
  queue_item_t *i;

  for (i = queue->front; i; i = i->next) {
  queue_item_destroy(i);
  }
  monitor_destroy(queue->workAvailable);
  free(queue);
}

typedef struct _libary {
  thread_t thread;
  queue_t *queue;
} library_t;

library_t *glibrary = NULL;


static void library_threadproc(void *param) {
  library_t *library = (library_t*)param;
  queue_item_t *item;
  for (item = queue_shift(library->queue); item; item = queue_shift(library->queue)) {
    if (item->type == QI_POISIONPILL) {
      queue_item_destroy(item);
      return;
    }

    if (!file_seek(item->stream->fd, item->offset)) {
      atomic_set(&item->stream->open, 0);
    }
    else if (item->type == QI_EOF) {
      file_seteof(item->stream->fd);
    }
    else if (!file_write(item->stream->fd, item->bytes, item->length)) {
      atomic_set(&item->stream->open, 0);
    }
    queue_item_destroy_stream(item);
  }
}

void delayed_stream_library_init() {
  if (glibrary) {
    return;
  }

  glibrary = (library_t *)malloc(sizeof(library_t));
  if (!glibrary) {
    abort();
  }
  glibrary->queue = queue_create();
  if (!glibrary->queue) {
    abort();
  }
  glibrary->thread = thread_create(library_threadproc, glibrary);
  if (glibrary->thread == NULL) {
    abort();
  }
}
void delayed_stream_library_finish() {
  if (!glibrary) {
    return;
  }
  queue_push(glibrary->queue, queue_item_create_poisonpill());
  thread_join(glibrary->thread);
  thread_destroy(glibrary->thread);
  queue_destroy(glibrary->queue);
  free(glibrary);
  glibrary = NULL;
}

void* delayed_stream_open(file_t file, PRInt64 size_hint) {
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
  stream->fd = file;
  if (!stream->fd) {
    goto error_cf;
  }
  stream->monitor = monitor_create();
  if (stream->monitor == NULL) {
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
  file_close(stream->fd);

error_cf:
  free(stream);
  pool_destroy(pool);
  return NULL;
}

int delayed_stream_write(stream_t *stream, PRInt64 offset, const char *bytes, size_t length) {
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
  monitor_enter(stream->monitor);
  while (stream->pending > 0) {
    monitor_join(stream->monitor);
  }
  monitor_leave(stream->monitor);
}

void delayed_stream_close(stream_t *stream) {
  if (!stream) {
    return;
  }

  // Close the stream
  atomic_set(&stream->open, 0);
  delayed_stream_flush(stream);

  file_close(stream->fd);
  monitor_destroy(stream->monitor);
  pool_destroy(stream->pool);
  free(stream);
}
