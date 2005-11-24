#ifndef __GLITE_GSOAP_PLUGIN_H__
#define __GLITE_GSOAP_PLUGIN_H__

#include <stdsoap2.h>

#include "glite_gss.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_ID		"GLITE_GSOAP_PLUGIN"

struct _glite_gsplugin_ctx {
	struct timeval			_timeout, *timeout;

	char				   *error_msg;

	char				   *key_filename;
	char				   *cert_filename;

	edg_wll_GssConnection  *connection;
	gss_cred_id_t			cred;

	void				   *user_data;
};

typedef struct _glite_gsplugin_ctx *glite_gsplugin_Context;

extern int glite_gsplugin_init_context(glite_gsplugin_Context *);
extern int glite_gsplugin_free_context(glite_gsplugin_Context);
extern glite_gsplugin_Context glite_gsplugin_get_context(struct soap *);
extern void *glite_gsplugin_get_udata(struct soap *);
extern void glite_gsplugin_set_udata(struct soap *, void *);

extern void glite_gsplugin_set_timeout(glite_gsplugin_Context, struct timeval const *);

extern int glite_gsplugin(struct soap *, struct soap_plugin *, void *);
extern char *glite_gsplugin_errdesc(struct soap *);

#ifdef __cplusplus
}
#endif

#endif
