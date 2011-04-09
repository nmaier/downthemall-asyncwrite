/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#include <windows.h>

#include "pool.h"
#include "plarena.h"

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
