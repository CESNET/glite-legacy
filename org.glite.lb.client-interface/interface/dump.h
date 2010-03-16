#ifndef __EDG_WORKLOAD_LOGGING_CLIENT_DUMP_H__
#define __EDG_WORKLOAD_LOGGING_CLIENT_DUMP_H__

#ident "$Header$"
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


#define EDG_WLL_DUMP_NOW	-1
#define EDG_WLL_DUMP_LAST_START	-2
#define EDG_WLL_DUMP_LAST_END	-3
/*      if adding new attribute, add conversion string to common/xml_conversions.c too !! */

typedef struct {
	time_t	from,to;
} edg_wll_DumpRequest;

typedef struct {
	char	*server_file;
	time_t	from,to;
} edg_wll_DumpResult;

/** Dump events in a given time interval
 */

int edg_wll_DumpEvents(
	edg_wll_Context,
	const edg_wll_DumpRequest *,
	edg_wll_DumpResult *
);


#endif

