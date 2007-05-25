#ifndef _GLITE_JOBID_H
#define _GLITE_JOBID_H

/*!
 * \file cjobid.h
 * \brief L&B consumer API
 */

#ident "$Header$"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _glite_lbu_JobId *glite_lbu_JobId;

#define GLITE_WMSC_JOBID_DEFAULT_PORT 9000 /**< Default port where bookkeeping server listens */
#define GLITE_WMSC_JOBID_PROTO_PREFIX "https://" /**< JobId protocol prefix */


/* All the pointer functions return malloc'ed objects (or NULL on error) */

/**
 * Create a Job ID.
 * See the lb_draft document for details on its construction and components
 * \param bkserver book keeping server hostname
 * \param port port for the bk service
 * \param jobid new created job id
 * \ret al 0 success
 * \retval EINVAL invalid bkserver
 * \retval ENOMEM if memory allocation fails
 */
int glite_lbu_JobIdCreate(const char * bkserver, int port, glite_lbu_JobId * jobid);

/**
 * Recreate a Job ID
 * \param bkserver bookkeeping server hostname
 * \param port port for the bk service
 * \param unique string which represent created jobid (if NULL then new
 * one is created)
 * \param jobid new created job id
 * \retval 0 success
 * \retval EINVAL invalid bkserver
 * \retval ENOMEM if memory allocation fails
 */
int glite_lbu_JobIdRecreate(const char *bkserver, int port, const char * unique, glite_lbu_JobId * jobid);

/**
 * Create copy of Job ID
 * \param in jobid for duplication
 * \param jobid  duplicated jobid
 * \retval 0 for success
 * \retval EINVAL invalid jobid
 * \retval ENOMEM if memory allocation fails
 */
int glite_lbu_JobIdDup(const glite_lbu_JobId in, glite_lbu_JobId * jobid);

/*
 * Free jobid structure
 * \param jobid for dealocation
 */
void glite_lbu_JobIdFree(glite_lbu_JobId jobid);

/**
 * Parse Job ID string and creates jobid structure
 * \param jobidstr string representation of jobid
 * \param jobid parsed job id
 * \retval 0 for success
 * \retval EINVAL jobidstr can't be parsed
 * \retval ENOMEM if memory allocation fails
 */
int glite_lbu_JobIdParse(const char* jobidstr, glite_lbu_JobId * jobid);

/**
 * Unparse Job ID (produce the string form of JobId).
 * \param jobid to be converted to string
 * \return allocated string which represents jobid
 */
char* glite_lbu_JobIdUnparse(const glite_lbu_JobId jobid);

/**
 * Extract bookkeeping server address (address:port)
 * \param jobid from which the bkserver address should be extracted
 * \retval pointer to allocated string with bkserver address
 * \retval NULL if jobid is 0 or memory allocation fails
 */
char* glite_lbu_JobIdGetServer(const glite_lbu_JobId jobid);

/**
 * Extract bookkeeping server address and port
 * \param jobid from which the bkserver address should be extracted
 * \param srvName pointer where to return server name
 * \param srvPort pointer where to return server port
 *     */
void glite_lbu_JobIdGetServerParts(const glite_lbu_JobId jobid, char **srvName, unsigned int *srvPort);

/**
 * Extract unique string 
 * \param jobid
 * \retval pointer to allocated unique string representing jobid
 * \retval NULL if jobid is 0 or memory allocation fails
 */
char* glite_lbu_JobIdGetUnique(const glite_lbu_JobId jobid);

#ifdef __cplusplus
}
#endif

#endif /* _GLITE_JOBID_H */
