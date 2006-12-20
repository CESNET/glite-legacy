#ifndef __EDG_WORKLOAD_LOGGING_CLIENT_PRODUCER_H__
#define __EDG_WORKLOAD_LOGGING_CLIENT_PRODUCER_H__

#ident "$Header$"

#ifdef __cplusplus
extern "C" {
#endif

#include "glite/lb/context.h"

/**
 * Register job with L&B service.
 * Done via logging REGJOB event, may generate subjob id's and create
 * the parent-children associations.
 * Set the job as current for the context and initialize sequence code.
 *
 * Partitionable jobs should set num_subjobs=0 initially,
 * and re-register when number of subjobs becomes known.
 *
 * \param[in,out] context 	context to work with
 * \param[in] job 		jobId
 * \param[in] type 		EDG_WLL_JOB_SIMPLE,  EDG_WLL_JOB_DAG, or EDG_WLL_JOB_PARTITIONABLE
 * \param[in] jdl 		user-specified JDL
 * \param[in] ns 		network server contact
 * \param[in] num_subjobs 	number of subjobs to create
 * \param[in] seed 		seed used for subjob id's generator.
 * 	Use non-NULL value to be able to regenerate the set of jobid's
 * \param[out] subjobs 		returned subjob id's
 */

/* backward compatibility */
#define EDG_WLL_JOB_SIMPLE	EDG_WLL_REGJOB_SIMPLE
 
extern int edg_wll_RegisterJob(
	glite_lb_ClntContext	context,
	const char *	job,
	...
/*
	enum edg_wll_RegJobJobtype	type,
	const char *		jdl,
	const char *		ns,
	int			num_subjobs,
	const char *		seed,
	glite_lbu_JobId **	subjobs
*/
);

/** 
 * Synchronous variant of edg_wll_RegisterJob
 */

extern int edg_wll_RegisterJobSync(
	edg_wll_Context		context,
	const glite_lbu_JobId	job,
	enum edg_wll_RegJobJobtype	type,
	const char *		jdl,
	const char *		ns,
	int			num_subjobs,
	const char *		seed,
	glite_lbu_JobId **	subjobs
);

/**
 * Register job with L&B Proxy service.
 * Done via logging REGJOB event, may generate subjob id's and create
 * the parent-children associations.
 * Set the job as current for the context and initialize sequence code.
 *
 * Partitionable jobs should set num_subjobs=0 initially,
 * and re-register when number of subjobs becomes known.
 *
 * \param[in] type 		EDG_WLL_JOB_SIMPLE, EDG_WLL_JOB_DAG, or EDG_WLL_JOB_PARTITIONABLE
 * \param[in] user 		user credentials
 * \param[in] jdl 		user-specified JDL
 * \param[in] ns 		network server contact
 * \param[in] num_subjobs 	number of subjobs to create
 * \param[in] seed 		seed used for subjob id's generator.
 *      Use non-NULL value to be able to regenerate the set of jobid's
 * \param[out] subjobs 		returned subjob id's
 */

extern int edg_wll_RegisterJobProxy(
	edg_wll_Context		context,
	const glite_lbu_JobId	job,
	enum edg_wll_RegJobJobtype	type,
	const char *		jdl,
	const char *		ns,
	int			num_subjobs,
	const char *		seed,
	glite_lbu_JobId **	subjobs
);

#ifdef LB_PERF
/* register only to LBProxy 		*/
/* useful for performance measurements	*/

extern int edg_wll_RegisterJobProxyOnly(
	edg_wll_Context		context,
	const glite_lbu_JobId	job,
	enum edg_wll_RegJobJobtype	type,
	const char *		jdl,
	const char *		ns,
	int			num_subjobs,
	const char *		seed,
	glite_lbu_JobId **	subjobs
);
#endif



/**
 * Register subjobs in a batch.
 * Mainly used to provide JDL's of individual subjobs in a more efficient
 * way than logging them one by one.
 * \param[in] jdls		array of JDL's
 * \param[in] subjobs 		array of jobid's in the same order
 */

extern int edg_wll_RegisterSubjobs(
	edg_wll_Context		context,
	const glite_lbu_JobId	parent,
	char const * const *	jdls,
	const char *		ns,
	glite_lbu_JobId const *	subjobs
);


/**
 * Register subjobs to LB Proxyin a batch.
 * Mainly used to provide JDL's of individual subjobs in a more efficient
 * way than logging them one by one.
 * \param[in] jdls		array of JDL's
 * \param[in] subjobs 		array of jobid's in the same order
 */

extern int edg_wll_RegisterSubjobsProxy(
	edg_wll_Context		context,
	const glite_lbu_JobId	parent,
	char const * const *	jdls,
	const char *		ns,
	glite_lbu_JobId const *	subjobs
);

/**
 * Generate or regenerate set of subjob ID's.
 * Calls the same algorithm used to generate subjob ID's in edg_wll_RegisterJob().
 * Local semantics only, server is not contacted.
 */

extern int edg_wll_GenerateSubjobIds(
	edg_wll_Context		context,
	const glite_lbu_JobId	parent,
	int			num_subjobs,
	const char *		seed,
	glite_lbu_JobId **	subjobs
);


enum edg_wll_Permission {
	EDG_WLL_PERM_READ  = 1,
	EDG_WLL_PERM_WRITE = 4,
	EDG_WLL_PERM_ADMIN = 8,
};

enum edg_wll_PermissionType {
	EDG_WLL_PERM_ALLOW,
	EDG_WLL_PERM_DENY,
};

enum edg_wll_ACLOperation {
	EDG_WLL_ACL_ADD,
	EDG_WLL_ACL_REMOVE,
};

enum edg_wll_UserIdType {
	EDG_WLL_USER_SUBJECT,		/* X.509 subject name */
	EDG_WLL_USER_VOMS_GROUP,	/* VOMS group membership */
};

/**
 * Change ACL for given job.
 * \param[in,out] context 	context to work with
 * \param[in] job 		jobId
 * \param[in] user_id 		specification of user's credential
 * \param[in] user_id_type 	type of user_id,
 *    for EDG_WLL_USER_SUBJECT the user_id parameter is expected to be user's subject name
 *    for EDG_WLL_USER_VOMS_GROUP the user_id is expected to be of the form VO:group specifying required group membersip as managed by VOMS
 * \param[in] permission 	ACL permission to change
 * \param[in] permission_type 	type of given permission (allow or deny operation) 
 * \param[in] operation 	operation to perform with ACL (add or remove record)
 */
 
extern int edg_wll_ChangeACL(
	edg_wll_Context		context,
	const glite_lbu_JobId	job,
	const char *		user_id,
	enum edg_wll_UserIdType	user_id_type,
	enum edg_wll_Permission		permission,
	enum edg_wll_PermissionType	permission_type,
	enum edg_wll_ACLOperation	operation
);


#ifdef __cplusplus
}
#endif

#endif /* __EDG_WORKLOAD_LOGGING_CLIENT_PRODUCER_H__ */
