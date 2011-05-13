/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once

typedef void* file_t /* PRFileDesc* */;

int file_seek(file_t file, __int64 offset);
int file_write(file_t file, void *buffer, int amount);
void file_seteof(file_t file);
void file_flush(file_t file);
void file_close(file_t file);
