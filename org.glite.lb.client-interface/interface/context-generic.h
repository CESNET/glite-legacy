#ifndef _EDG_WORKLOAD_LOGGING_CLIENT_CONTEXT_H
#define _EDG_WORKLOAD_LOGGING_CLIENT_CONTEXT_H

/**
 * \file context.h
 * \brief L&B API common context (publicly visible) and related definitions
 */

#ident "$Header$"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup context Context
 *
 *@{
 */

/** Opaque context type */
typedef struct _glite_lb_ClntContext *glite_lb_ClntContext;

/** Constants defining the context parameters */
typedef enum _glite_lb_ContextParam {
	GLITE_LB_PARAM_HOST,		/**< hostname to appear as event orgin */
/* XXX: changed to string */
	GLITE_LB_PARAM_SOURCE,		/**< event source component */
	GLITE_LB_PARAM_INSTANCE,	/**< instance of the source component */

	/* unused EDG_WLL_PARAM_LEVEL, 	**< logging level */

	GLITE_LB_PARAM_DESTINATION,	/**< logging destination host */
	GLITE_LB_PARAM_DESTINATION_PORT, /**< logging destination port */
	GLITE_LB_PARAM_LOG_TIMEOUT,	/**< logging timeout (asynchronous) */
	GLITE_LB_PARAM_LOG_SYNC_TIMEOUT,	/**< logging timeout (synchronous) */

/* TODO: consumer */
	EDG_WLL_PARAM_QUERY_SERVER,	/**< default server name to query */
	EDG_WLL_PARAM_QUERY_SERVER_PORT,/**< default server port to query */
	EDG_WLL_PARAM_QUERY_SERVER_OVERRIDE,/**< host:port parameter setting override even values in jobid (useful for debugging & hacking only) */
	EDG_WLL_PARAM_QUERY_TIMEOUT,	/**< query timeout */
	EDG_WLL_PARAM_QUERY_JOBS_LIMIT,	/**< maximal query jobs result size */
	EDG_WLL_PARAM_QUERY_EVENTS_LIMIT,/**< maximal query events result size */
	EDG_WLL_PARAM_QUERY_RESULTS,	/**< maximal query result size */
	EDG_WLL_PARAM_QUERY_CONNECTIONS,/**< maximal number of open connections in ctx->connPoll */
	EDG_WLL_PARAM_NOTIF_SERVER,	/**< default notification server name */
	EDG_WLL_PARAM_NOTIF_SERVER_PORT,/**< default notification server port */
	EDG_WLL_PARAM_NOTIF_TIMEOUT,	/**< notif timeout */
	
	GLITE_LB_PARAM_X509_PROXY,	/**< proxy file to use for authentication */
	GLITE_LB_PARAM_X509_KEY,		/**< key file to use for authentication */
	GLITE_LB_PARAM_X509_CERT,	/**< certificate file to use for authentication */
	GLITE_LB_PARAM_LBPROXY_STORE_SOCK,/**< lbproxy store socket path */
	GLITE_LB_PARAM_LBPROXY_SERVE_SOCK,/**<  lbproxy serve socket path */
	/* used interally only GLITE_LB_PARAM_LBPROXY_USER,	**< user credentials when logging to L&B Proxy */

/* FIXME: server only */
	EDG_WLL_PARAM_JPREG_TMPDIR,		/**< maildir storage path */

	GLITE_LB_PARAM__LAST,		/**< marker, LB internal use only */
} glite_lb_ContextParam;

/** Allocate an initialize a new context object.
 * \param[out] context 		returned context
 * \return 0 on success, ENOMEM if malloc() fails
 */
int glite_lb_InitContext(glite_lb_ClntContext *context);

/** Destroy and free context object.
 * Also performs necessary cleanup (closing connections etc.)
 * \param[in] context 		context to free
 */
void glite_lb_FreeContext(glite_lb_ClntContext context);

/** Set a context parameter.
 * \param[in,out] context 	context to work with
 * \param[in] param 		parameter to set
 * \param[in] ... 		value to set (if NULL or 0, default is used)
 * \retval 0 success
 * \retval EINVAL param is not a valid parameter, or invalid value
 */
int glite_lb_SetParam(
	glite_lb_ClntContext context,
	glite_lb_ContextParam param,
	...
);

struct timeval; /* XXX: gcc, shut up! */

/** Set a context parameter of type int.
 * \param[in,out] ctx           context to work with
 * \param[in] param		parameter to set
 * \param[in] val		value to set
 * \retval 0 success
 * \retval EINVAL param is not a valid parameter, or invalid value
 */
int glite_lb_SetParamInt(glite_lb_ClntContext ctx,glite_lb_ContextParam param,int val);

/** Set a context parameter of type string.
 * \param[in,out] ctx		context to work with
 * \param[in] param		parameter to set
 * \param[in] val		value to set (if NULL, default is used)
 * \retval 0 success
 * \retval EINVAL param is not a valid parameter, or invalid value
 */
int glite_lb_SetParamString(glite_lb_ClntContext ctx,glite_lb_ContextParam param,const char *val);

/** Set a context parameter of type timeval.
 * \param[in,out] ctx		context to work with
 * \param[in] param		parameter to set
 * \param[in] val		value to set (if NULL, default is used)
 * \retval 0 success
 * \retval EINVAL param is not a valid parameter, or invalid value
 */
int glite_lb_SetParamTime(glite_lb_ClntContext ctx,glite_lb_ContextParam param,const struct timeval *val);

/** Get current parameter value.
 * \param[in,out] context 	context to work with
 * \param[in] param 		parameter to retrieve
 * \param[out] ... 		pointer to output variable
 * \retval 0 success
 * \retval EINVAL param is not a valid parameter
 */
int glite_lb_GetParam(
	edg_wll_ClntContext context,
	edg_wll_ContextParam param,
	...
);


/**
 * L&B subsystem specific error codes.
 * Besides them L&B functions return standard \a errno codes in their usual
 * meaning.
 */

/* XXX: cleanup required */

#ifndef GLITE_WMS_LOGGING_ERROR_BASE
#define GLITE_WMS_LOGGING_ERROR_BASE 1400
#endif

/* TODO: error handling */

typedef enum _edg_wll_ErrorCode {
/** Base for L&B specific code. Use the constant from common/ */
	EDG_WLL_ERROR_BASE = GLITE_WMS_LOGGING_ERROR_BASE,
	EDG_WLL_ERROR_PARSE_BROKEN_ULM, /**< Parsing ULM line into edg_wll_Event structure */
	EDG_WLL_ERROR_PARSE_EVENT_UNDEF, /**< Undefined event name */
	EDG_WLL_ERROR_PARSE_MSG_INCOMPLETE, /**< Incomplete message (missing fields) */
	EDG_WLL_ERROR_PARSE_KEY_DUPLICITY, /**< Duplicate entry in message */
	EDG_WLL_ERROR_PARSE_KEY_MISUSE, /**< Entry not allowed for this message type */
	EDG_WLL_ERROR_PARSE_OK_WITH_EXTRA_FIELDS, /**< Additional, not understood fields found in message.
		The rest is OK therefore this is not a true error. */
        EDG_WLL_ERROR_XML_PARSE, /**< Error in parsing XML protocol. */
        EDG_WLL_ERROR_SERVER_RESPONSE, /**< Generic failure on server. See syslog on the server machine for details. */
        EDG_WLL_ERROR_JOBID_FORMAT, /**< Malformed jobid */
	EDG_WLL_ERROR_DB_CALL, /**< Failure of underlying database engine.
		See errDesc returned by edg_wll_ErrorCode(). */
	EDG_WLL_ERROR_URL_FORMAT, /**< Malformed URL */
	EDG_WLL_ERROR_MD5_CLASH, /**< MD5 hash same for different strings. Very unlikely :-). */
	EDG_WLL_ERROR_GSS, /**< Generic GSSAPI error. See errDesc returned by edg_wll_Error(). */
	EDG_WLL_ERROR_DNS, /**< DNS resolver error. See errDesc returned by edg_wll_Error(). */
	EDG_WLL_ERROR_NOJOBID,	/**< Attmepted call requires calling edg_wll_SetLoggingJob() first. */
	EDG_WLL_ERROR_NOINDEX,	/**< Query does not contain any conidion on indexed attribute. */
	EDG_WLL_IL_PROTO,	/**< Lbserver (proxy) store communication protocol error. */
	EDG_WLL_IL_SYS,         /**< Interlogger internal error. */
	EDG_WLL_IL_EVENTS_WAITING, /**< Interlogger still has events pending delivery. */
	EDG_WLL_ERROR_COMPARE_EVENTS, /**< Two compared events differ. */
	EDG_WLL_ERROR_SQL_PARSE, /**< Error in SQL parsing. */
} edg_wll_ErrorCode;

/**
 * Retrieve error details on recent API call
 * \param[in] context 		context to work with
 * \param[out] errText		standard error text
 *      (may be NULL - no text returned)
 * \param[out] errDesc		additional error description
 *      (may be NULL - no text returned)
 * \return Error code of the recent error
 */

int edg_wll_Error(
	edg_wll_Context context,
	char		**errText,
	char		**errDesc
);

/**
 * retrieve the current logging JobId from the context
 */
int glite_lb_GetLoggingJob(
	const glite_lb_ClntContext	context,
	char 	**jobid_out
);

/**
 * Set a current job for given context.
 * \note Should be called before any logging call.
 * \param[in,out] context 	context to work with
 * \param[in] job 		further logging calls are related to this job
 * \param[in] code 		sequence code as obtained from previous component
 * \param[in] flags 		flags on code handling (\see API documentation)
 */
extern int glite_lb_SetLoggingJob(
	glite_lb_ClntContext	context,
	char *jobid,
	...
/* flesh
	const char *		code,
	int			flags
*/
);

/**
 * Set a current job for given context.
 * \note Should be called before any logging call.
 * \param[in,out] context 	context to work with
 * \param[in] job 		further logging calls are related to this job
 * \param[in] code 		sequence code as obtained from previous component
 * \param[in] user 		user credentials
 * \param[in] flags 		flags on code handling (\see API documentation)
 */
extern int glite_lb_SetLoggingJobProxy(
	glite_lb_ClntContext	context,
	char * 			jobid,
	const char *		user,	/* XXX: changed arg order */
	...
/* flesh
	const char *		code,
	int			flags
*/
);


/*
 *@} end of group
 */

#ifdef __cplusplus
}
#endif

#endif 
