/*
 * This is a "glue" file containing various pieces that are needed to build
 * separated renewal_core library.
 */

#include <glite/security/voms/voms_apic.h>

#include "renewal_core.h"
#include "renewal_locl.h"
#include "renewd_locl.h"

char *vomsconf = NULL;
static glite_renewal_core_context saved_ctx = NULL;

/* Wrapper calling renew_voms_creds() from voms.o */
int
glite_renewal_renew_voms_creds(glite_renewal_core_context ctx, const char *cur_file, const char *renewed_file, const char *new_file)
{
   int ret;

   vomsconf = ctx->voms_conf;
   saved_ctx = ctx;
   ret = renew_voms_creds(cur_file, renewed_file, new_file);
   saved_ctx = NULL;

   return ret;
}

/* This is called from voms.o, normaly it sits in renew.o, which we don't want
 * to add to the renewal_core lib. */
int
load_proxy(const char *cur_file, X509 **cert, EVP_PKEY **priv_key,
           STACK_OF(X509) **chain, globus_gsi_cred_handle_t *cur_proxy)
{
   assert(saved_ctx != NULL);

   return glite_renewal_load_proxy(saved_ctx, cur_file, cert, priv_key, chain, cur_proxy);
}

/* This is called from voms.o */
void
edg_wlpr_Log(int dbg_level, const char *format, ...)
{
   char *str;
   va_list ap;

   assert(saved_ctx != NULL);

   va_start(ap, format);
   vasprintf(&str, format, ap);
   va_end(ap);

   glite_renewal_log(saved_ctx, dbg_level, "%s", str);
   free(str);
}

/* A new call not implemented in 3.0 version of voms.o */
int
glite_renewal_check_voms_attrs(glite_renewal_core_context ctx, const char *proxy)
{
   int ret, voms_err, present;
   X509 *cert = NULL;
   STACK_OF(X509) *chain = NULL;
   struct vomsdata *vd = NULL;

   ret = glite_renewal_load_proxy(ctx, proxy, &cert, NULL, &chain, NULL);
   if (ret)
      return 0;

   vd = VOMS_Init(NULL, NULL);
   if (vd == NULL) {
      present = 0;
      goto end;
   }

   ret = VOMS_Retrieve(cert, chain, RECURSE_CHAIN, vd, &voms_err);
   if (ret == 0) {
      present = 0;
      goto end;
   }

   present = 1;

end:
   if (cert)
      X509_free(cert);
   if (chain)
      sk_X509_pop_free(chain, X509_free);
   if (vd)
      VOMS_Destroy(vd);

   return present;
}
