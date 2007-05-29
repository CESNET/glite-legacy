#ifndef GLITE_SECURITY_GSOAP_PLUGIN_INTERNAL_H
#define GLITE_SECURITY_GSOAP_PLUGIN_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "glite/security/glite_gss.h"

struct _glite_gsplugin_ctx {
	struct timeval			_timeout, *timeout;

	char				   *error_msg;

	char				   *key_filename;
	char				   *cert_filename;

	edg_wll_GssConnection  *connection;
	gss_cred_id_t			cred;

	void				   *user_data;
};


#ifdef __cplusplus
}
#endif

#endif
