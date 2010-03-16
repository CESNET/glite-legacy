/*
Copyright (c) Members of the EGEE Collaboration. 2004-2010.
See http://www.eu-egee.org/partners for details on the copyright holders.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/* 
 * fake implementation of the producer API
 */

#ifndef WORKLOAD_LOGGING_CLIENT_PRODUCER_FAKE_H
#define WORKLOAD_LOGGING_CLIENT_PRODUCER_FAKE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "glite/lb/context.h"

typedef int (edg_wll_Logging_cb_f)(edg_wll_Context context);

int edg_wll_RegisterTestLogging(edg_wll_Logging_cb_f *cb);
int edg_wll_RegisterTestLoggingProxy(edg_wll_Logging_cb_f *cb);
void edg_wll_UnregisterTestLogging();
void edg_wll_UnregisterTestLoggingProxy();

#ifdef __cplusplus
}
#endif

#endif /* WORKLOAD_LOGGING_CLIENT_PRODUCER_FAKE_H */
