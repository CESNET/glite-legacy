#ifndef __EDG_WORKLOAD_LOGGING_COMMON_NOTIFID_H__
#define __EDG_WORKLOAD_LOGGING_COMMON_NOTIFID_H__

#ident "$Header$"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup notifid Notification Id (NotifId)
 * \brief NotifId description and handling.
 *@{
 */

/** Notification handle.
 * Refers to a particular registration for receiving notifications.
 */
typedef void *edg_wll_NotifId;

/**
 * Create a Job ID.
 * \param server	IN: notification server hostname
 * \param port		IN: port of the notification server
 * \param notifid	OUT: newly created NotifId
 * \retval 0 for success
 * \retval EINVAL invalid notification server
 * \retval ENOMEM if memory allocation fails
 */
int edg_wll_NotifIdCreate(const char *server, int port ,edg_wll_NotifId *notifid);

/**
 * Free the NotifId structure
 * \param notifid 	IN: for dealocation
 */
void edg_wll_NotifIdFree(edg_wll_NotifId notifid);

/** Parse the NotifId string and creates NotifId structure
 * \param notifidstr	IN: string representation of NotifId
 * \param notifid	OUT: parsed NotifId
 * \retval 0 for success
 * \retval EINVAL notifidstr can't be parsed
 * \retval ENOMEM if memory allocation fails
 */
int edg_wll_NotifIdParse(const char *notifidstr, edg_wll_NotifId *notifid);

/** Unparse the NotifId (produce the string form of NotifId).
 * \param notifid	IN: NotifId to be converted to string
 * \return allocated string which represents the NotifId
 */
char* edg_wll_NotifIdUnparse(const edg_wll_NotifId notifid);

/**
 * Extract notification server address (address:port)
 * \param notifid 	IN: NotifId from which the address should be extracted
 * \param srvName	INOUT: pointer where to return server name
 * \param srvPort	INOUT: pointer where to return server port
 */
void edg_wll_NotifIdGetServerParts(const edg_wll_NotifId notifid, char **srvName, unsigned int *srvPort);

/**
 * Extract unique string 
 * \param notifid	IN: NotifId 
 * \retval pointer to allocated unique string representing jobid
 * \retval NULL if jobid is 0 or memory allocation fails
 */ 
char *edg_wll_NotifIdGetUnique(const edg_wll_NotifId notifid);

/**
 * Recreate a NotifId by a new unique string
 * \param unique	IN: string which represent created notifid (if NULL then new
 * one is created)
 * \param notifid       INOUT: newly created NotifId 
 * \retval 0 success
 * \retval EINVAL invalid NotifId
 * \retval ENOMEM if memory allocation fails
 */
int edg_wll_NotifIdSetUnique(edg_wll_NotifId *notifid, const char *unique);

/*
 *@} end of group
 */

#ifdef __cplusplus
}
#endif
#endif
