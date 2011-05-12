/************************************************************************
 * To the extent possible under law, Nils Maier has waived all copyright
 * and related or neighboring rights to this work.
 ************************************************************************/

#pragma once

typedef void* event_t;
typedef int event_result_t;

#define EVENT_WAIT_INFINITE 0
#define EVENT_RESULT_TIMEOUT 0
#define EVENT_RESULT_SUCCESS 1
#define EVENT_RESULT_ERROR 2

event_t event_create();
event_result_t event_wait(event_t event, unsigned int timeout);
void event_set(event_t event);
void event_destroy(event_t event);
