#ifndef __EDG_WORKLOAD_LOGGING_CLIENT_PURGE_H__
#define __EDG_WORKLOAD_LOGGING_CLIENT_PURGE_H__

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


/** Purge or dump request */
typedef struct _edg_wll_PurgeRequest {
	char	**jobs;		/**< list of jobid's to work on */ 

/** Purge jobs that are in the given states and "untouched" at least for the
  * specified interval.
  * Currently applicable for CLEARED, ABORTED, CANCELLED and OTHER (catchall).
  * The other array members are for future extensions.
  * Negative values stand for server defaults.
  */
	time_t	timeout[EDG_WLL_NUMBER_OF_STATCODES];
#define EDG_WLL_PURGE_JOBSTAT_OTHER	EDG_WLL_JOB_UNDEF


/**
 * Actions to be taken and information required.
 */
	int	flags;

/** no dry run */
#define EDG_WLL_PURGE_REALLY_PURGE	1
/** return list of jobid matching the purge/dump criteria */
#define EDG_WLL_PURGE_LIST_JOBS		2
/** dump to a file on the sever */
#define EDG_WLL_PURGE_SERVER_DUMP	4
/** TODO: stream the dump info to the client */
#define EDG_WLL_PURGE_CLIENT_DUMP	8
/* ! when addning new constant, add it also to common/xml_conversions.c ! */

	
/** private request processing data (for the reentrant functions) */
/* TODO */
	
} edg_wll_PurgeRequest;

/** Output data of a purge or dump */
typedef struct _edg_wll_PurgeResult {
	char	*server_file;	/**< filename of the dump at the server */
	char	**jobs;		/**< affected jobs */
/* TODO: output of the streaming interface */
} edg_wll_PurgeResult;


/** Client side purge/dump 
 * \retval EAGAIN only partial result returned, call repeatedly to get all
 * 	output data
 */
int edg_wll_Purge(
	edg_wll_Context ctx,
	edg_wll_PurgeRequest *request,
	edg_wll_PurgeResult	*result
);

#endif
