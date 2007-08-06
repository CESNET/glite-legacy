#ifndef __EDG_WORKLOAD_LOGGING_COMMON_LB_GSS_H__
#define __EDG_WORKLOAD_LOGGING_COMMON_LB_GSS_H__

#ident "$Header$"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

enum {
  EDG_WLL_GSS_OK		=  0,  /* no GSS errors */
  EDG_WLL_GSS_ERROR_GSS		= -1,  /* GSS specific error, call edg_wll_get_gss_error() for details */
  EDG_WLL_GSS_ERROR_TIMEOUT	= -2,  /* Timeout */
  EDG_WLL_GSS_ERROR_EOF		= -3,  /* EOF occured */
  EDG_WLL_GSS_ERROR_ERRNO	= -4,  /* System error. See errno */
  EDG_WLL_GSS_ERROR_HERRNO	= -5   /* Resolver error. See h_errno */
};

enum {
  EDG_WLL_GSS_FLAG_DELEG = 1,
  EDG_WLL_GSS_FLAG_CONF = 16,
  EDG_WLL_GSS_FLAG_INTEG = 32,
  EDG_WLL_GSS_FLAG_ANON = 64,
};

typedef void * edg_wll_GssName;
typedef void * edg_wll_GssCtx;
typedef void * edg_wll_GssCred;

typedef struct _edg_wll_GssConnection {
  edg_wll_GssCtx context;
  int sock;
  char *buffer;
  size_t bufsize;
} edg_wll_GssConnection;

typedef struct _edg_wll_GssStatus {
  unsigned int major_status;
  unsigned int minor_status;
} edg_wll_GssStatus;

typedef struct _edg_wll_GssPrincipal_data {
   char *name;
   unsigned int flags;
#if 0
   char **fqans;
   char **voms_groups; /* needed for legacy LB server authZ mechanism */
   edg_wll_GssOid authn_mech;
#endif
} edg_wll_GssPrincipal_data;
typedef struct _edg_wll_GssPrincipal_data *edg_wll_GssPrincipal;

int
edg_wll_gss_initialize(void);

int
edg_wll_gss_acquire_cred_gsi(const char *cert_file,
		             const char *key_file,
		             edg_wll_GssCred *cred,
		             char **name,
			     edg_wll_GssStatus* gss_code);

int
edg_wll_gss_release_cred(edg_wll_GssCred cred,
			 edg_wll_GssStatus* gss_code);

int 
edg_wll_gss_connect(edg_wll_GssCred cred,
		    char const *hostname,
		    int port,
		    struct timeval *timeout,
		    edg_wll_GssConnection *connection,
		    edg_wll_GssStatus* gss_code);

int
edg_wll_gss_accept(edg_wll_GssCred cred,
		   int sock,
		   struct timeval *timeout,
		   edg_wll_GssConnection *connection,
		   edg_wll_GssStatus* gss_code);

int
edg_wll_gss_read(edg_wll_GssConnection *connection,
		 void *buf,
	         size_t bufsize,
		 struct timeval *timeout,
		 edg_wll_GssStatus* gss_code);

int
edg_wll_gss_write(edg_wll_GssConnection *connection,
		  const void *buf,
		  size_t bufsize,
		  struct timeval *timeout,
		  edg_wll_GssStatus* gss_code);

int
edg_wll_gss_read_full(edg_wll_GssConnection *connection,
		      void *buf,
		      size_t bufsize,
		      struct timeval *timeout,
		      size_t *total,
		      edg_wll_GssStatus* gss_code);

int
edg_wll_gss_write_full(edg_wll_GssConnection *connection,
		       const void *buf,
		       size_t bufsize,
		       struct timeval *timeout,
		       size_t *total,
		       edg_wll_GssStatus* gss_code);

int
edg_wll_gss_watch_creds(const char * proxy_file,
			time_t * proxy_mtime);

int
edg_wll_gss_get_error(edg_wll_GssStatus* gss_code,
		      const char *prefix,
		      char **errmsg);

int
edg_wll_gss_close(edg_wll_GssConnection *connection,
		  struct timeval *timeout);

int 
edg_wll_gss_reject(int sock);

int
edg_wll_gss_get_client_conn(edg_wll_GssConnection *connection,
	  	            edg_wll_GssPrincipal *principal,
			    edg_wll_GssStatus* gss_code);

void
edg_wll_gss_free_princ(edg_wll_GssPrincipal principal);

int
edg_wll_gss_gethostname(char *name, int len);

#ifdef __cplusplus
} 
#endif
	
#endif /* __EDG_WORKLOAD_LOGGING_COMMON_LB_GSS_H__ */
