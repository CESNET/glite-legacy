/* $Id: */

#ifndef WORKLOAD_LOGGING_CLIENT_CONSUMER_FAKE_H
#define WORKLOAD_LOGGING_CLIENT_CONSUMER_FAKE_H

#include "glite/lb/context-int.h"

typedef int (edg_wll_QueryEvents_cb_f)(edg_wll_Context context, edg_wll_Event **events);
typedef int (edg_wll_QueryListener_cb_f)(edg_wll_Context context, char **host, uint16_t *port);

int egd_wll_RegisterTestQueryEvents(edg_wll_QueryEvents_cb_f *cb);
int egd_wll_RegisterTestQueryListener(edg_wll_QueryListener_cb_f *cb);
void egd_wll_UnregisterTestQueryEvents();
void egd_wll_UnregisterTestQueryListener();

#endif /* WORKLOAD_LOGGING_CLIENT_CONSUMER_FAKE_H */
