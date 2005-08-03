#ifndef __EDG_WORKLOAD_LOGGING_CLIENT_CONSUMER_H__
#define __EDG_WORKLOAD_LOGGING_CLIENT_CONSUMER_H__

/*!
 * \file consumer.h
 * \brief L&B consumer API
 */

#ident "$Header$"

#include "glite/wmsutils/jobid/cjobid.h"
#include "glite/lb/context.h"
#include "glite/lb/events.h"
#include "glite/lb/jobstat.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup querying Server querying
 * \brief The core part of the LB querying API.
 * 
 * The functions in this part of the API are responsible for
 * transforming the user query to the LB protocol, contacting the server,
 * receiving back the response and transforming back the results to the
 * API data structures. 
 *
 * General rules:
 * - functions return 0 on success, nonzero on error, errror details can 
 *   be found via edg_wll_ErrorCode()
 * - OUT are ** types, functions malloc()-ate objects and fill in the pointer 
 *   pointed to by the OUT argument
 * - returned lists of pointers are NULL-terminated malloc()-ed arrays
 * - edg_wll_Query + wrapper terminate arrays with EDG_WLL_EVENT_UNDEF event
 * - OUT is NULL if the list is empty
 *@{
 */

/**
 * Predefined types for query attributes
 */
typedef enum _edg_wll_QueryAttr{
	EDG_WLL_QUERY_ATTR_UNDEF=0,	/**< Not-defined value, used to terminate lists etc. */
	EDG_WLL_QUERY_ATTR_JOBID,	/**< Job Id \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_ATTR_OWNER,	/**< Job owner \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_ATTR_STATUS,	/**< Current job status */
	EDG_WLL_QUERY_ATTR_LOCATION,	/**< Where is the job processed */
	EDG_WLL_QUERY_ATTR_DESTINATION,	/**< Destination CE */
	EDG_WLL_QUERY_ATTR_DONECODE,	/**< Minor done status (OK,fail,cancel) */
	EDG_WLL_QUERY_ATTR_USERTAG,	/**< User tag */
	EDG_WLL_QUERY_ATTR_TIME,	/**< Timestamp \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_ATTR_LEVEL,	/**< Logging level (see "dglog.h") * \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_ATTR_HOST,	/**< Where the event was generated */
	EDG_WLL_QUERY_ATTR_SOURCE,	/**< Source component */
	EDG_WLL_QUERY_ATTR_INSTANCE,	/**< Instance of the source component */
	EDG_WLL_QUERY_ATTR_EVENT_TYPE,	/**< Event type \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_ATTR_CHKPT_TAG,	/**< Checkpoint tag */
	EDG_WLL_QUERY_ATTR_RESUBMITTED,	/**< Job was resubmitted */
	EDG_WLL_QUERY_ATTR_PARENT,	/**< Job was resubmitted */
	EDG_WLL_QUERY_ATTR_EXITCODE,	/**< Unix exit code */
	EDG_WLL_QUERY_ATTR__LAST
/*	if adding new attribute, add conversion string to common/xml_conversions.c too !! */
} edg_wll_QueryAttr;


/**
 * Predefined types for query operands
 */
typedef enum _edg_wll_QueryOp{
	EDG_WLL_QUERY_OP_EQUAL,		/**< attribute is equal to the operand value \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_OP_LESS,		/**< attribute is grater than the operand value \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_OP_GREATER,	/**< attribute is less than the operand value \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_OP_WITHIN,	/**< attribute is in given interval \see _edg_wll_QueryRec */
	EDG_WLL_QUERY_OP_UNEQUAL,	/**< attribute is not equal to the operand value \see _edg_wll_QueryRec */
} edg_wll_QueryOp;


/**
 * Single query condition for edg_wll_Query().
 * Those records are composed to form an SQL \a where clause
 * when processed at the L&B server
 */
typedef struct _edg_wll_QueryRec {
	edg_wll_QueryAttr	attr;	/**< attribute to query */
	edg_wll_QueryOp		op;	/**< query operation */

/**
 * Specification of attribute to query
 */
	union {
		char *			tag;	/**< user tag name */
		edg_wll_JobStatCode	state;	/**< job status code */	
	} attr_id;
/**
 * Query operand.
 * The appropriate type is uniquely defined by the attr member
 */
	union edg_wll_QueryVal {
		int	i;	/**< integer query attribute value */
		char	*c;	/**< character query attribute value */
		struct timeval	t;	/**< time query attribute value */
		edg_wlc_JobId	j;	/**< JobId query attribute value */
	} value, value2;
} edg_wll_QueryRec;

/**
 * default query timeout (in seconds)
 */
#define EDG_WLL_QUERY_TIMEOUT_DEFAULT   120

/**
 * maximal query timeout (in seconds)
 */
#define EDG_WLL_QUERY_TIMEOUT_MAX       1800

/**
 * General query on events.
 * Return events satisfying all conditions 
 * query records represent conditions in the form 
 * \a attr \a op \a value eg. time > 87654321.
 * \see edg_wll_QueryRec
 *
 * \param[in] context 		context to work with
 * \param[in] job_conditions 	query conditions (ANDed) on current job status, null (i.e. ATTR_UNDEF) terminated list. NULL means empty list, i.e. always TRUE
 * \param[in] event_conditions 	conditions on events, null terminated list, NULL means empty list, i.e. always TRUE
 * \param[out] events 		list of matching events
 */
int edg_wll_QueryEvents(
	edg_wll_Context			context,
	const edg_wll_QueryRec *	job_conditions,
	const edg_wll_QueryRec *	event_conditions,
	edg_wll_Event **		events
);

/**
 * Extended event query interface.
 * Similar to \ref edg_wll_QueryEvents but the conditions are nested lists.
 * Elements of the inner lists have to refer to the same attribute and they
 * are logically ORed. 
 * The inner lists themselves are logically ANDed then.
 */

int edg_wll_QueryEventsExt(
	edg_wll_Context			context,
	const edg_wll_QueryRec **	job_conditions,
	const edg_wll_QueryRec **	event_conditions,
	edg_wll_Event **		events
);


/**
 * Query LBProxy and use plain communication
 * \warning edg_wll_*Proxy() functions are not implemented in release 1.
 */
int edg_wll_QueryEventsProxy(
	edg_wll_Context			context,
	const edg_wll_QueryRec *	job_conditions,
	const edg_wll_QueryRec *	event_conditions,
	edg_wll_Event **		events
);

/**
 * \warning edg_wll_*Proxy() functions are not implemented in release 1.
 */

int edg_wll_QueryEventsExtProxy(
	edg_wll_Context			context,
	const edg_wll_QueryRec **	job_conditions,
	const edg_wll_QueryRec **	event_conditions,
	edg_wll_Event **		events
);

/** 
 * General query on jobs.
 * Return jobs (and possibly their states) for which an event satisfying the conditions
 * exists.
 * \see edg_wll_QueryEvents
 * \param[in] context 		context to work with
 * \param[in] conditions 	query records (ANDed), null (i.e. EDG_WLL_ATTR_UNDEF) terminated list
 * \param[in] flags 		additional status fields to retrieve (\see edg_wll_JobStatus)
 * \param[out] jobs 		list of job ids. May be NULL.
 * \param[out] states 		list of corresponding states (returned only if not NULL)
 */
int edg_wll_QueryJobs(
	edg_wll_Context			context,
	const edg_wll_QueryRec *	conditions,
	int				flags,
	edg_wlc_JobId **		jobs,
	edg_wll_JobStat **		states
);

/**
 * Extended job query interface.
 * Similar to \ref edg_wll_QueryJobs but the conditions are nested lists.
 * Elements of the inner lists have to refer to the same attribute and they
 * are logically ORed. 
 * The inner lists themselves are logically ANDed then.
 */


int edg_wll_QueryJobsExt(
	edg_wll_Context			context,
	const edg_wll_QueryRec **	conditions,
	int				flags,
	edg_wlc_JobId **		jobs,
	edg_wll_JobStat **		states
);


/**
 * Query LBProxy and use plain communication
 * \warning edg_wll_*Proxy() functions are not implemented in release 1.
 */
int edg_wll_QueryJobsProxy(
	edg_wll_Context			context,
	const edg_wll_QueryRec *	conditions,
	int				flags,
	edg_wlc_JobId **		jobs,
	edg_wll_JobStat **		states
);

/**
 * \warning edg_wll_*Proxy() functions are not implemented in release 1.
 */

int edg_wll_QueryJobsExtProxy(
	edg_wll_Context			context,
	const edg_wll_QueryRec **	conditions,
	int				flags,
	edg_wlc_JobId **		jobs,
	edg_wll_JobStat **		states
);


/**
 * Bitmasks for edg_wll_JobStatus() flags argument.
 * Settings these flags causes the status calls to retrieve additional
 * information.
 */
#define EDG_WLL_STAT_CLASSADS	1	/**< various job description fields */
#define EDG_WLL_STAT_CHILDREN	2	/**< list of subjob JobId's */
#define EDG_WLL_STAT_CHILDSTAT	4	/**< apply the flags recursively to subjobs */
/* starting from bit 10 private flags begins - do not add 1024 and more! */

/** Return status of a single job.
 * \param[in] context 		context to operate on
 * \param[in] jobid 		query this job
 * \param[in] flags 		specifies optional status fields to retrieve,
 * 	\see EDG_WLL_STAT_CLASSADS, EDG_WLL_STAT_CHILDREN, EDG_WLL_STAT_CHILDSTAT
 * \param[out] status		status
 */

int edg_wll_JobStatus(
	edg_wll_Context		context,
	const edg_wlc_JobId		jobid,
	int			flags,
	edg_wll_JobStat		*status
);

/**
 * Query LBProxy and use plain communication
 * \param[in] context 		context to operate on
 * \param[in] jobid 		query this job
 * \param[in] flags 		specifies optional status fields to retrieve,
 *     \see EDG_WLL_STAT_CLASSADS, EDG_WLL_STAT_CHILDREN, EDG_WLL_STAT_CHILDSTAT
 * \param[out] status 		the status of the job

 * \warning edg_wll_*Proxy() functions are not implemented in release 1.
 */
int edg_wll_JobStatusProxy(
	edg_wll_Context		context,
	const edg_wlc_JobId	jobid,
	int			flags,
	edg_wll_JobStat		*status
);

/**
 * Return all events related to a single job.
 * Convenience wrapper around edg_wll_Query()
 * \param[in] context 		context to work with
 * \param[in] jobId 		job to query
 * \param[out] events 		list of events 
 */

int edg_wll_JobLog(
	edg_wll_Context		context,
	const edg_wlc_JobId	jobId,
	edg_wll_Event **	events
);


/**
 * Query LBProxy and use plain communication
 * \warning edg_wll_*Proxy() functions are not implemented in release 1.

 */
int edg_wll_JobLogProxy(
	edg_wll_Context		context,
	const edg_wlc_JobId	jobId,
	edg_wll_Event **	events
);

/**
 * All current user's jobs.
 * \param[in] context 		context to work with
 * \param[out] jobs 		list of the user's jobs
 * \param[out] states 		list of the jobs' states
 */
int edg_wll_UserJobs(
	edg_wll_Context		context,
	edg_wlc_JobId **	jobs,
	edg_wll_JobStat	**	states
);


/**
 * Query LBProxy and use plain communication
 * \warning edg_wll_*Proxy() functions are not implemented in release 1.

 */
int edg_wll_UserJobsProxy(
	edg_wll_Context		context,
	edg_wlc_JobId **	jobs,
	edg_wll_JobStat	**	states
);

/**
 * Server supported indexed attributes
 * \see DataGrid-01-TEN-0125
 * \param[in] context 		context to work with
 * \param[out] attrs 		configured indices (each index is an UNDEF-terminated
 * 		array of QueryRec's from which only attr (and attr_id 
 * 		eventually) are meaningful
 */
int edg_wll_GetIndexedAttrs(
	edg_wll_Context		context,
	edg_wll_QueryRec	***attrs
);

/**
 * Retrieve limit on query result size (no. of events or jobs).
 * \warning not implemented.
 * \see DataGrid-01-TEN-0125
 * \param[in] context 		context to work with
 * \param[out] limit 		server imposed limit
 */
int edg_wll_GetServerLimit(
	edg_wll_Context	context,
	int		*limit
);

/**
 * UI port for intactive jobs. Used internally by WMS.
 * \param[in] context 		context to work with
 * \param[in] jobId 		job to query
 * \param[in] name 		name of the UI-port
 * \param[out] host 		hostname of port
 * \param[out] port 		port number
 */
int edg_wll_QueryListener(
	edg_wll_Context	context,
	edg_wlc_JobId		jobId,
	const char *	name,
	char **		host,
	uint16_t *	port
);


/**
 * Query LBProxy and use plain communication
 */
int edg_wll_QueryListenerProxy(
	edg_wll_Context	context,
	edg_wlc_JobId		jobId,
	const char *	name,
	char **		host,
	uint16_t *	port
);

/**
 * Ask LB Proxy server for sequence number
 * \param[in] context 		context to work with
 * \param[in] jobId 		job to query
 * \param[out] code 		sequence code
 */


int edg_wll_QuerySequenceCodeProxy(
	edg_wll_Context	context,
	edg_wlc_JobId	jobId,
	char **		code
);
		
/*
 * edg_wll_QueryRec manipulation
 */

/** Free edg_wll_QueryRec internals, not the structure itself */
void edg_wll_QueryRecFree(edg_wll_QueryRec *);

/*
 *@} end of group
 */

#ifdef CLIENT_SBIN_PROG
extern int edg_wll_http_send_recv(
	edg_wll_Context,
	char *, const char * const *, char *,
	char **,char ***,char **
);

extern int http_check_status(
	edg_wll_Context,
	char *,
	char **
);

extern int set_server_name_and_port(
	edg_wll_Context,
	const edg_wll_QueryRec **
);

#endif

/**
 * default query timeout (in seconds)
 */
#define EDG_WLL_QUERY_TIMEOUT_DEFAULT   120

/**
 * maximal query timeout (in seconds)
 */
#define EDG_WLL_QUERY_TIMEOUT_MAX       1800

#ifdef __cplusplus
}
#endif

#endif /* __EDG_WORKLOAD_LOGGING_CLIENT_CONSUMER_H__ */
