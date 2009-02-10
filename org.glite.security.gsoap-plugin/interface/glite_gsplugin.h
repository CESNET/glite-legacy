#ifndef GLITE_SECURITY_GSOAP_PLUGIN_H
#define GLITE_SECURITY_GSOAP_PLUGIN_H

#include <stdsoap2.h>

#include "glite/security/glite_gss.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PLUGIN_ID		"GLITE_GSOAP_PLUGIN"

typedef struct _glite_gsplugin_ctx *glite_gsplugin_Context;

extern int glite_gsplugin_init_context(glite_gsplugin_Context *);
extern int glite_gsplugin_free_context(glite_gsplugin_Context);
extern glite_gsplugin_Context glite_gsplugin_get_context(struct soap *);
extern void *glite_gsplugin_get_udata(struct soap *);
extern void glite_gsplugin_set_udata(struct soap *, void *);

extern void glite_gsplugin_set_timeout(glite_gsplugin_Context, struct timeval const *);
extern int glite_gsplugin_set_credential(glite_gsplugin_Context, const char *, const char *);
extern void glite_gsplugin_use_credential(glite_gsplugin_Context, edg_wll_GssCred);
extern int glite_gsplugin_set_connection(glite_gsplugin_Context, edg_wll_GssConnection *);

extern int glite_gsplugin(struct soap *, struct soap_plugin *, void *);
extern char *glite_gsplugin_errdesc(struct soap *);

#ifdef __cplusplus
}
#endif

#endif /* GLITE_SECURITY_GSOAP_PLUGIN_H */
