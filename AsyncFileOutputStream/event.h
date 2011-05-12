/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once

typedef void* event_t;

event_t event_create();
int event_join(event_t event);
void event_set(event_t event);
void event_destroy(event_t event);
