#ifndef __EDG_WORKLOAD_LOGGING_CLIENT_NOTIFICATION_H__
#define __EDG_WORKLOAD_LOGGING_CLIENT_NOTIFICATION_H__

#ident "$Header$"

#include "glite/wmsutils/jobid/cjobid.h"
#include "glite/lb/notifid.h"
#include "glite/lb/context.h"
#include "glite/lb/consumer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup notifications Notifications handling
 * \brief Notifications handling.
 *@{
 */

/**
 * default and maximal notif timeout (in seconds)
 */
#define EDG_WLL_NOTIF_TIMEOUT_DEFAULT   120
#define EDG_WLL_NOTIF_TIMEOUT_MAX       1800


/** Register for receiving notifications.
 * Connects to the server specified by EDG_WLL_NOTIF_SERVER context parameter
 * (temporary workaround, should be resolved by registry in future).
 * \param context	INOUT: context to work with
 * \param conditions	IN: the same conditions as for \ref edg_wll_QueryJobsExt.
 * 		currently one or more JOBID's are required.
 * 		Only a single occurence of a specific attribute is allowed
 * 		among ANDed conditions (due to the ability to modify them
 * 		further).
 * \param fd		IN: = -1 create or reuse the default listening socket (one per context)
 * 	     >= 0 non-default listening socket 
 * \param address_override IN: if not NULL, use this address instead of extracting it
 * 		from the connection (useful when multiple interfaces are present,
 * 		circumventing NAT problems etc.)
 * \param valid 	IN: until when the registration is valid (NULL means no interest in
 * \param id_out	OUT: returened NotifId
 * 		the value
 * \retval 0 OK
 * \retval EINVAL restrictions on conditions are not met
 * 
 */
int edg_wll_NotifNew(
	edg_wll_Context		context,
	edg_wll_QueryRec	const * const *conditions,
	int			fd,
	const char		*address_override,
	edg_wll_NotifId		*id_out,
	time_t			*valid
);


/** Change the receiving local address.
 * Report the new address to the server.
 *
 * \param context	INOUT: context to work with
 * \param id		IN: notification ID you are binding to
 * \param fd		IN; same as for \ref edg_wll_NotifNew 
 * \param address_override IN: same as for \ref edg_wll_NotifNew
 * \param valid 	IN: same as for \ref edg_wll_NotifNew
 */

int edg_wll_NotifBind(
	edg_wll_Context		context,
	const edg_wll_NotifId	id,
	int			fd,
	const char		*address_override,
	time_t			*valid
);

typedef enum _edg_wll_NotifChangeOp {
	/** No operation, equal to not defined */
	EDG_WLL_NOTIF_NOOP = 0,
	/** Replace notification registration with new one */
	EDG_WLL_NOTIF_REPLACE,
	/** Add new condition when to be notifed */
	EDG_WLL_NOTIF_ADD,
	/** Remove condition on notification */
	EDG_WLL_NOTIF_REMOVE
/*      if adding new attribute, add conversion string to common/xml_conversions.c too !! */
} edg_wll_NotifChangeOp;

/** Modify the query conditions for this notification.
 *
 * If op is either EDG_WLL_NOTIF_ADD or EDG_WLL_NOTIF_REMOVE, for the sake
 * of uniqueness the original conditions must have contained only a single
 * OR-ed row of conditions on the attributes infolved in the change.
 * 
 * \param context	INOUT: context to work with
 * \param id		IN: notification ID you are working with
 * \param conditions	IN: same as for \ref edg_wll_NotifNew
 * \param op 		IN: action to be taken on existing conditions,
 * 	\ref edg_wll_NotifChangeOp
 */
int edg_wll_NotifChange(
	edg_wll_Context		context,
	const edg_wll_NotifId	id,
	edg_wll_QueryRec	const * const * conditions,
	edg_wll_NotifChangeOp	op
);

/** Refresh the registration, i.e. extend its validity period.
 * \param context	INOUT: context to work with
 * \param id		IN: notification ID you are working with
 * \param valid 	IN: until when the registration is valid (NULL means no interest in
 * 		the value
 */

int edg_wll_NotifRefresh(
	edg_wll_Context		context,
	const edg_wll_NotifId	id,
	time_t			*valid
);

/** Drop the registration.
 * Server is instructed not to send notifications anymore, pending ones
 * are discarded, listening socket is closed, and allocated memory freed.
 * \param context	INOUT: context to work with
 * \param id		IN: notification ID you are working with
 */

int edg_wll_NotifDrop(
	edg_wll_Context		context,
	edg_wll_NotifId		*id
);

/** Receive notification.
 * The first incoming notification is returned.
 * \param context	INOUT: context to work with
 * \param fd 		IN: receive on this socket (-1 means the default for the context)
 * \param timeout 	IN: wait atmost this time long. (0,0) means polling, NULL waiting
 * 	indefinitely
 * \param state_out	OUT: returned JobStatus
 * \param id_out	OUT: returned NotifId
 * \retval 0 notification received, state_out contains the current job state
 * \retval EAGAIN no notification available, timeout occured
 */

int edg_wll_NotifReceive(
	edg_wll_Context		context,
	int			fd,
	const struct timeval	*timeout,
	edg_wll_JobStat		*state_out,
	edg_wll_NotifId		*id_out
);


/** Default socket descriptor where to select(2) for notifications.
 * Even if nothing is available for reading from the socket, 
 * there may be some data cached so calling \ref edg_wll_NotifReceive
 * may return notifications immediately.
 *
 * \param context	INOUT: context to work with
 * \retval >=0 socket descriptor
 * \retval -1 error, details set in context
 */

int edg_wll_NotifGetFd(
	edg_wll_Context		context
);

/** Close the default local listening socket.
 * Useful to force following \ref edg_wll_NotifBind to open
 * a new one.
 * \param context	INOUT: context to work with
 */

int edg_wll_NotifCloseFd(
	edg_wll_Context		context
);

/*
 *@} end of group
 */

#ifdef __cplusplus
}
#endif

#endif
