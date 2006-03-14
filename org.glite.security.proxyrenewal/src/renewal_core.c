#include "renewal_core.h"
#include "renewal_locl.h"
#include "renewd_locl.h"

static const char rcsid[] = "$Id$";

int
load_proxy(const char *cur_file, X509 **cert, EVP_PKEY **priv_key,
           STACK_OF(X509) **chain, globus_gsi_cred_handle_t *cur_proxy)
{
   globus_result_t result;
   globus_gsi_cred_handle_t proxy = NULL;
   int ret;

   result = globus_gsi_cred_handle_init(&proxy, NULL);
   if (result) {
      fprintf(stderr, "globus_gsi_cred_handle_init() failed\n");
      goto end;
   }

   result = globus_gsi_cred_read_proxy(proxy, cur_file);
   if (result) {
      fprintf(stderr, "globus_gsi_cred_read_proxy() failed\n");
      goto end;
   }

   if (cert) {
      result = globus_gsi_cred_get_cert(proxy, cert);
      if (result) {
	 fprintf(stderr, "globus_gsi_cred_get_cert() failed\n");
	 goto end;
      }
   }

   if (priv_key) {
      result = globus_gsi_cred_get_key(proxy, priv_key);
      if (result) {
	 fprintf(stderr, "globus_gsi_cred_get_key() failed\n");
	 goto end;
      }
   }

   if (chain) {
      result = globus_gsi_cred_get_cert_chain(proxy, chain);
      if (result) {
	 fprintf(stderr, "globus_gsi_cred_get_cert_chain() failed\n");
	 goto end;
      }
   }

   if (cur_proxy) {
      *cur_proxy = proxy;
      proxy = NULL;
   }

   ret = 0;
   
end:
   if (proxy)
      globus_gsi_cred_handle_destroy(proxy);
   if (result)
      ret = EDG_WLPR_ERROR_GENERIC;

   return ret;
}

int
get_proxy_base_name(char *file, char **name)
{
   X509 *cert = NULL;
   EVP_PKEY *key = NULL;
   STACK_OF(X509) *chain = NULL;
   X509_NAME *subject = NULL;
   int ret;

   ret = load_proxy(file, &cert, &key, &chain, NULL);
   if (ret)
      return ret;

   subject = X509_NAME_dup(X509_get_subject_name(cert));

   sk_X509_insert(chain, cert, 0);
   cert = NULL;

   ret = globus_gsi_cert_utils_get_base_name(subject, chain);
   if (ret) {
      edg_wlpr_Log(LOG_ERR, "Cannot get subject name from proxy %s", file);
      ret = EDG_WLPR_ERROR_SSL; /* XXX ??? */
      goto end;
   }

   *name = X509_NAME_oneline(subject, NULL, 0);
   ret = 0;

end:
   if (cert)
      X509_free(cert);
   if (key)
      EVP_PKEY_free(key);
   if (chain)
      sk_X509_pop_free(chain, X509_free);
   if (subject)
      X509_NAME_free(subject);

   return ret;
}

int
glite_renewal_core_renew(glite_renewal_core_context context,
                         const char * myproxy_server,
			 unsigned int myproxy_port,
                         const char *current_proxy,
                         char **new_proxy)
{
   char tmp_proxy[FILENAME_MAX];
   int tmp_fd;
   int ret = -1;
   char *p;
   char *server = NULL;
   myproxy_socket_attrs_t *socket_attrs;
   myproxy_request_t      *client_request;
   myproxy_response_t     *server_response;
   char *renewed_proxy;
   /* XXX */
   int voms_exts = 1;

   socket_attrs = malloc(sizeof(*socket_attrs));
   memset(socket_attrs, 0, sizeof(*socket_attrs));

   client_request = malloc(sizeof(*client_request));
   memset(client_request, 0, sizeof(*client_request));

   server_response = malloc(sizeof(*server_response));
   memset(server_response, 0, sizeof(*server_response));

   myproxy_set_delegation_defaults(socket_attrs, client_request);

   edg_wlpr_Log(LOG_DEBUG, "Trying to renew proxy in %s", current_proxy);

   snprintf(tmp_proxy, sizeof(tmp_proxy), "%s.myproxy.XXXXXX", current_proxy);
   tmp_fd = mkstemp(tmp_proxy);
   if (tmp_fd == -1) {
      edg_wlpr_Log(LOG_ERR, "Cannot create temporary file (%s)",
                   strerror(errno));
      return errno;
   }

   ret = get_proxy_base_name(current_proxy, &client_request->username);
   if (ret)
      goto end;

   client_request->proxy_lifetime = 60 * 60 * DGPR_RETRIEVE_DEFAULT_HOURS;

   server = (myproxy_server) ? myproxy_server : socket_attrs->pshost;
   if (server == NULL) {
      edg_wlpr_Log(LOG_ERR, "No myproxy server specified");
      ret = EINVAL;
      goto end;
   }
   socket_attrs->pshost = strdup(server);

   socket_attrs->psport = (myproxy_port) ? myproxy_port : MYPROXY_SERVER_PORT;

   verror_clear();
   ret = myproxy_get_delegation(socket_attrs, client_request, current_proxy,
	                        server_response, tmp_proxy);
   if (ret == 1) {
      ret = EDG_WLPR_ERROR_MYPROXY;
      edg_wlpr_Log(LOG_ERR, "Error contacting MyProxy server for proxy %s: %s",
	           current_proxy, verror_get_string());
      verror_clear();
      goto end;
   }

   renewed_proxy = tmp_proxy;

   if (context->voms_enabled && voms_exts) {
      char tmp_voms_proxy[FILENAME_MAX];
      int tmp_voms_fd;
      
      snprintf(tmp_voms_proxy, sizeof(tmp_voms_proxy), "%s.voms.XXXXXX",
	       current_proxy);
      tmp_voms_fd = mkstemp(tmp_voms_proxy);
      if (tmp_voms_fd == -1) {
	 edg_wlpr_Log(LOG_ERR, "Cannot create temporary file (%s)",
	              strerror(errno));
	 ret = errno;
	 goto end;
      }

      ret = renew_voms_creds(current_proxy, renewed_proxy, tmp_voms_proxy);
      close(tmp_voms_fd);
      if (ret) {
	 unlink(tmp_voms_proxy);
	 goto end;
      }

      renewed_proxy = tmp_voms_proxy;
      unlink(tmp_proxy);
   }

   if (new_proxy)
      *new_proxy = strdup(renewed_proxy);

   ret = 0;

end:
   if (socket_attrs->socket_fd)
      close(socket_attrs->socket_fd);
   close(tmp_fd);
   if (ret)
      unlink(tmp_proxy);
   myproxy_free(socket_attrs, client_request, server_response);

   return ret;
}

int
glite_renewal_core_init_ctx(glite_renewal_core_context *context)
{
   glite_renewal_core_context p = NULL;

   *context = NULL;

   p = calloc(1, sizeof(*p));
   if (p == NULL)
      return ENOMEM;

   *context = p;
   return 0;
}

int
glite_renewal_core_destroy_ctx(glite_renewal_core_context context)
{
   free(context);
   return 0;
}

/* XXX remove these ugly things: */

void
edg_wlpr_Log(int dbg_level, const char *format, ...)
{
   return;
}

char *vomsconf = NULL;
