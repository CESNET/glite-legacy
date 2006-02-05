#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <stdsoap2.h>

#include "soap_version.h"
#include "glite_gsplugin.h"

#ifdef GSPLUGIN_DEBUG
#  define pdprintf(s)	printf s
#else
#  define pdprintf(s)
#endif

typedef struct _int_plugin_data_t {
	glite_gsplugin_Context	ctx;	/**< data used for connection etc. */
	int						def;	/**< is the context created by plugin? */
} int_plugin_data_t;

static const char plugin_id[] = PLUGIN_ID;

static void glite_gsplugin_delete(struct soap *, struct soap_plugin *);
static int glite_gsplugin_copy(struct soap *, struct soap_plugin *, struct soap_plugin *);

static size_t glite_gsplugin_recv(struct soap *, char *, size_t);
static int glite_gsplugin_send(struct soap *, const char *, size_t);
static int glite_gsplugin_connect(struct soap *, const char *, const char *, int);
static int glite_gsplugin_close(struct soap *);
static int glite_gsplugin_accept(struct soap *, int, struct sockaddr *, int *);


int
glite_gsplugin_init_context(glite_gsplugin_Context *ctx)
{
	glite_gsplugin_Context out = (glite_gsplugin_Context) malloc(sizeof(*out));
	if (!out) return ENOMEM;

	memset(out, 0, sizeof(*out));
	out->cred = GSS_C_NO_CREDENTIAL;

	/* XXX: some troubles with glite_gss and blocking calls!
	out->timeout.tv_sec = 10000;
	 */

	out->timeout = NULL;
	*ctx = out;

	return 0;
}

int
glite_gsplugin_free_context(glite_gsplugin_Context ctx)
{
	OM_uint32	ms;

	if ( ctx->cred != GSS_C_NO_CREDENTIAL ) gss_release_cred(&ms, &ctx->cred);
	if ( ctx->connection ) {
		if ( ctx->connection->context != GSS_C_NO_CONTEXT )
			edg_wll_gss_close(ctx->connection, NULL);
		free(ctx->connection);
	}
	free(ctx->error_msg);
	free(ctx->key_filename);
	free(ctx->cert_filename);
	free(ctx);

	return 0;
}

glite_gsplugin_Context
glite_gsplugin_get_context(struct soap *soap)
{
	return ((int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id))->ctx;
}

void *
glite_gsplugin_get_udata(struct soap *soap)
{
	int_plugin_data_t *pdata;
   
	pdata = (int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id);
	assert(pdata);
	return pdata->ctx->user_data;
}

void
glite_gsplugin_set_udata(struct soap *soap, void *d)
{
	int_plugin_data_t *pdata;
   
	pdata = (int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id);
	assert(pdata);
	pdata->ctx->user_data = d;
}

void glite_gsplugin_set_timeout(glite_gsplugin_Context ctx, struct timeval const *to)
{
	if (to) {
		ctx->_timeout = *to;
		ctx->timeout = &ctx->_timeout;
	}
	else ctx->timeout = NULL;
}

int
glite_gsplugin_set_credential(glite_gsplugin_Context ctx,
			      const char *cert,
			      const char *key)
{
   edg_wll_GssStatus gss_code;
   int ret;

   ret = edg_wll_gss_acquire_cred_gsi(cert, key, &ctx->cred, NULL, &gss_code);
   if (ret) {
      /* XXX propagate error description */
      return EINVAL;
   }

   ctx->cert_filename = cert;
   ctx->key_filename = key;

   return 0;
}

int
glite_gsplugin(struct soap *soap, struct soap_plugin *p, void *arg)
{
	int_plugin_data_t *pdata = malloc(sizeof(int_plugin_data_t)); 

	pdprintf(("GSLITE_GSPLUGIN: initializing gSOAP plugin\n"));
	if ( !pdata ) return ENOMEM;
	if ( arg ) {
		pdprintf(("GSLITE_GSPLUGIN: Context is given\n"));
		pdata->ctx = arg;
		pdata->def = 0;
	}
	else {
		edg_wll_GssStatus	gss_code;
		char			   *subject = NULL;

		pdprintf(("GSLITE_GSPLUGIN: Creating default context\n"));
		if ( glite_gsplugin_init_context((glite_gsplugin_Context*)&(pdata->ctx)) ) {
			free(pdata);
			return ENOMEM;
		}
		if ( edg_wll_gss_acquire_cred_gsi(NULL, NULL, &pdata->ctx->cred, &subject, &gss_code) ) {
			/*	XXX: Let user know, that cred. load failed. Somehow...
			 */
			glite_gsplugin_free_context(pdata->ctx);
			return EINVAL;
		}
		pdprintf(("GSLITE_GSPLUGIN: server running with certificate: %s\n", subject));
		free(subject);
	}

	p->id			= plugin_id;
	p->data			= pdata;
	p->fdelete		= glite_gsplugin_delete;
	p->fcopy		= glite_gsplugin_copy;

	soap->fconnect		= glite_gsplugin_connect;
	soap->fclose		= glite_gsplugin_close;
#if GSOAP_VERSION >= 20700
	soap->fclosesocket	= glite_gsplugin_close;
#endif
	soap->faccept		= glite_gsplugin_accept;
	soap->fsend			= glite_gsplugin_send;
	soap->frecv			= glite_gsplugin_recv;


	return SOAP_OK;
}


char *glite_gsplugin_errdesc(struct soap *soap)
{
	glite_gsplugin_Context	ctx;

	ctx = ((int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id))->ctx;
	if ( ctx ) return ctx->error_msg;

	return NULL;
}



static int
glite_gsplugin_copy(struct soap *soap, struct soap_plugin *dst, struct soap_plugin *src)
{
	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_copy()\n"));
	/*	Should be the copy code here?
	 */
	return ENOSYS;
}

static void
glite_gsplugin_delete(struct soap *soap, struct soap_plugin *p)
{
	int_plugin_data_t *d = (int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id);

	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_delete()\n"));
	if ( d->def ) {
		OM_uint32	ms;

		glite_gsplugin_close(soap);
		if (d->ctx->cred != GSS_C_NO_CREDENTIAL) gss_release_cred(&ms, &d->ctx->cred);
		free(d->ctx->error_msg);
	}
	free(d);
}


static int
glite_gsplugin_connect(
	struct soap *soap,
	const char *endpoint,
	const char *host,
	int port)
{
	glite_gsplugin_Context	ctx;
	edg_wll_GssStatus		gss_stat;
	int						ret;


	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_connect()\n"));
#if defined(CHECK_GSOAP_VERSION) && GSOAP_VERSION <= 20700
	if (   GSOAP_VERSION < 20700
		|| (GSOAP_VERSION == 20700
			&& (strlen(GSOAP_MIN_VERSION) < 1 || GSOAP_MIN_VERSION[1] < 'e')) ) {
		fprintf(stderr, "Client connect will work only with gSOAP v2.7.0e and later");
		return ENOSYS;
	}
#endif

	ctx = ((int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id))->ctx;

	if ( ctx->cred == GSS_C_NO_CREDENTIAL ) {
		pdprintf(("GSLITE_GSPLUGIN: loading credentials\n"));
		ret = edg_wll_gss_acquire_cred_gsi(ctx->cert_filename, ctx->key_filename,
						&ctx->cred, NULL, &gss_stat);
		if ( ret ) {
			edg_wll_gss_get_error(&gss_stat, "failed to load GSI credentials",
						&ctx->error_msg);
			goto err;
		}
	}

	if ( !(ctx->connection = malloc(sizeof(*ctx->connection))) ) return errno;
	ret = edg_wll_gss_connect(ctx->cred,
				host, port,
				ctx->timeout,
				ctx->connection, &gss_stat);
	if ( ret ) {
		free(ctx->connection);
		ctx->connection = NULL;
		edg_wll_gss_get_error(&gss_stat, "edg_wll_gss_connect()", &ctx->error_msg);
		goto err;
	}


err:
	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_connect() error!\n"));
	switch ( ret ) {
	case EDG_WLL_GSS_ERROR_HERRNO: 
	case EDG_WLL_GSS_ERROR_ERRNO: return errno;
	case EDG_WLL_GSS_ERROR_EOF: return ECONNREFUSED;
	case EDG_WLL_GSS_ERROR_TIMEOUT: return ETIMEDOUT;
	}

	return ret;
}

/**	It is called in soap_closesocket() 
 *
 *	return like errno value
 */
static int
glite_gsplugin_close(struct soap *soap)
{
	glite_gsplugin_Context	ctx;
	int						ret = SOAP_OK;


	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_close()\n"));
	ctx = ((int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id))->ctx;
	if ( ctx->connection ) {
		if ( ctx->connection->context != GSS_C_NO_CONTEXT) {
			pdprintf(("GSLITE_GSPLUGIN: closing gss connection\n"));
			ret = edg_wll_gss_close(ctx->connection, ctx->timeout);
		}
		ctx->connection->context = GSS_C_NO_CONTEXT;
	}

	return ret;
}


static int
glite_gsplugin_accept(struct soap *soap, int s, struct sockaddr *a, int *n)
{
	glite_gsplugin_Context	ctx;
	edg_wll_GssStatus		gss_code;
	int						conn;


	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_accept()\n"));
	ctx = ((int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id))->ctx;
	if ( (conn = accept(s, (struct sockaddr *)&a, n)) < 0 ) return conn;
	if (   !ctx->connection
		&& !(ctx->connection = malloc(sizeof(*ctx->connection))) ) return -1;
	if ( edg_wll_gss_accept(ctx->cred, conn, ctx->timeout, ctx->connection, &gss_code)) {
		pdprintf(("GSLITE_GSPLUGIN: Client authentication failed, closing.\n"));
		edg_wll_gss_get_error(&gss_code, "Client authentication failed", &ctx->error_msg);
		return -1;
	}

	return conn;
}

static size_t
glite_gsplugin_recv(struct soap *soap, char *buf, size_t bufsz)
{
	glite_gsplugin_Context	ctx;
	edg_wll_GssStatus		gss_code;
	int						len;


	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_recv()\n"));
	ctx = ((int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id))->ctx;
	if ( ctx->error_msg ) { free(ctx->error_msg); ctx->error_msg = NULL; }

	if ( ctx->connection->context == GSS_C_NO_CONTEXT ) {
		soap->errnum = ENOTCONN;
		/* XXX: glite_gsplugin_send() returns SOAP_EOF on errors
		 */
		return 0;
	}
	
	len = edg_wll_gss_read(ctx->connection, buf, bufsz,
					ctx->timeout,
					&gss_code);

	switch ( len ) {
	case EDG_WLL_GSS_OK:
		break;

	case EDG_WLL_GSS_ERROR_GSS:
		edg_wll_gss_get_error(&gss_code, "receving WS request",
		      		      &ctx->error_msg);
		soap->errnum = ENOTCONN;
		return 0;

	case EDG_WLL_GSS_ERROR_ERRNO:
		ctx->error_msg = strdup("edg_wll_gss_read()");
		soap->errnum = errno;
		return 0;

	case EDG_WLL_GSS_ERROR_TIMEOUT:
		soap->errnum = ETIMEDOUT;
		return 0;

	case EDG_WLL_GSS_ERROR_EOF:
		soap->errnum = ENOTCONN;
		return 0;

		/* default: fallthrough */
	}

	return len;
}

static int
glite_gsplugin_send(struct soap *soap, const char *buf, size_t bufsz)
{
	glite_gsplugin_Context	ctx;
	edg_wll_GssStatus		gss_code;
	struct sigaction		sa, osa;
	size_t					total = 0;
	int						ret;


	pdprintf(("GSLITE_GSPLUGIN: glite_gsplugin_send()\n"));
	ctx = ((int_plugin_data_t *)soap_lookup_plugin(soap, plugin_id))->ctx;
	/* XXX: check whether ctx is initialized
	 *      i.e. ctx->connection != NULL
	 */
	if ( ctx->error_msg ) { free(ctx->error_msg); ctx->error_msg = NULL; }
	if ( ctx->connection->context == GSS_C_NO_CONTEXT ) {
		soap->errnum = ENOTCONN;
		return SOAP_EOF;
	}

	memset(&sa, 0, sizeof(sa));
	assert(sa.sa_handler == NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, &osa);

	ret = edg_wll_gss_write_full(ctx->connection,
				(void*)buf, bufsz, ctx->timeout, &total, &gss_code);

	sigaction(SIGPIPE, &osa, NULL);

	switch ( ret ) {
	case EDG_WLL_GSS_OK:
		break;

	case EDG_WLL_GSS_ERROR_TIMEOUT:
		ctx->error_msg = strdup("glite_gsplugin_send()");
		soap->errnum = ETIMEDOUT;
		return SOAP_EOF;

	case EDG_WLL_GSS_ERROR_ERRNO:
		if ( errno == EPIPE ) {
			ctx->error_msg = strdup("glite_gsplugin_send()");
			soap->errnum = ENOTCONN;
		}
		else {
			ctx->error_msg = strdup("glite_gsplugin_send()");
			soap->errnum = errno;
		}
		return SOAP_EOF;

	case EDG_WLL_GSS_ERROR_GSS:
	case EDG_WLL_GSS_ERROR_EOF:
	default:
		ctx->error_msg = strdup("glite_gsplugin_send()");
		soap->errnum = ENOTCONN;
		return SOAP_EOF;
	}

	return SOAP_OK;
}

