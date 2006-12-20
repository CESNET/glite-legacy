#ifndef _EDG_WORKLOAD_LOGGING_CLIENT_CONTEXT_H
#define _EDG_WORKLOAD_LOGGING_CLIENT_CONTEXT_H

/**
 * \file context.h
 * \brief L&B API common context (publicly visible) and related definitions
 */

#include "glite/lb/context-generic.h"

#ident "$Header$"

#ifdef __cplusplus
extern "C" {
#endif

#define GLITE_LB_LEGACY_SOURCE_USER_INTERFACE "UserInterface"
#define GLITE_LB_LEGACY_SOURCE_NETWORK_SERVER "NetworkServer"
#define GLITE_LB_LEGACY_SOURCE_WORKLOAD_MANAGER "WorkloadManager"
#define GLITE_LB_LEGACY_SOURCE_BIG_HELPER "BigHelper"
#define GLITE_LB_LEGACY_SOURCE_JOB_SUBMISSION "JobSubmission"
#define GLITE_LB_LEGACY_SOURCE_LOG_MONITOR "LogMonitor"
#define GLITE_LB_LEGACY_SOURCE_LRMS "LRMS"
#define GLITE_LB_LEGACY_SOURCE_APPLICATION "Application"

/** Currently an alias. Will be replaced when migration NS -> WMProxy
 * is finished. */
#define GLITE_LB_LEGACY_SOURCE_WM_PROXY GLITE_LB_LEGACY_SOURCE_NETWORK_SERVER

/**
 * type of sequence code (used when setting to the context)
 */
#define GLITE_LB_LEGACY_SEQ_NORMAL      1
#define GLITE_LB_LEGACY_SEQ_DUPLICATE   11

/**
 * initial sequence code for BigHelper
 */
#define GLITE_LB_LEGACY_SEQ_BIGHELPER_INITIAL "UI=2:NS=0:WM=0:BH=1:JSS=0:LM=0:LRMS=0:APP=0"

/** Retrieve current sequence code from the context */
char * glite_lb_Legacy_GetSequenceCode(
	const glite_lb_ClntContext	context
);

/*
 *@} end of group
 */

#ifdef __cplusplus
}
#endif

#endif 
