#ifndef __GLITE_GSOAP_PLUGIN_H__
#define __GLITE_GSOAP_PLUGIN_H__

#include <stdsoap2.h>

#include "lb_gss.h"

#define PLUGIN_ID		"GLITE_GSOAP_PLUGIN"

struct _glite_gsplugin_ctx {
	struct timeval			timeout; /**<	timeout for all netcalls
									  *		if tv_sec ==0 and tv_usec == 0 then
									  *		no timeout is used - every call will
									  *		be considered as a nonblocking */

	char				   *error_msg;

	char				   *key_filename;
	char				   *cert_filename;

	edg_wll_GssConnection  *connection;
	gss_cred_id_t			cred;
};

typedef struct _glite_gsplugin_ctx *glite_gsplugin_Context;

extern int glite_gsplugin_init_context(glite_gsplugin_Context *);

extern int glite_gsplugin(struct soap *, struct soap_plugin *, void *);
extern char *glite_gsplugin_errdesc(struct soap *);

#endif
