#ifndef RENEWALD_LOCL_H
#define RENEWALD_LOCL_H

#ident "$Header$"

#include <myproxy.h>
#include <myproxy_delegation.h>

#include "glite/wmsutils/thirdparty/globus_ssl_utils/sslutils.h"
#include "renewal.h"

#ifdef HAVE_DMALLOC_H
#include <dmalloc.h>
#endif

/* XXX */
#if 0
#define EDG_WLPR_ERROR_PARSE_NOT_FOUND EDG_WLPR_ERROR_PROTO_PARSE_ERROR
#define EDG_WLPR_ERROR_NOTFOUND        EDG_WLPR_PROXY_NOT_REGISTERED
#endif

typedef struct {
   unsigned int len;
   char **val;
} prd_list;

typedef struct {
   int suffix;
   prd_list jobids;
   int unique;
   int voms_exts;
   char *myproxy_server;
   time_t end_time;
   time_t next_renewal;
} proxy_record;

typedef struct vomses_record {
   char *nick;
   char *hostname;
   int port;
} vomses_record;

typedef struct vomses_records {
   unsigned int len;
   struct vomses_record **val;
} vomses_records;

/* commands */
void
register_proxy(edg_wlpr_Request *request, edg_wlpr_Response *response);

void
unregister_proxy(edg_wlpr_Request *request, edg_wlpr_Response *response);

void
get_proxy(edg_wlpr_Request *request, edg_wlpr_Response *response);

void
update_db(edg_wlpr_Request *request, edg_wlpr_Response *response);

int
get_times(char *proxy_file, proxy_record *record);

void
watchdog_start(void);

void
edg_wlpr_Log(int dbg_level, const char *format, ...);

int
decode_record(char *line, proxy_record *record);

int
encode_record(proxy_record *record, char **line);

void
free_record(proxy_record *record);

int
load_proxy(const char *filename, X509 **cert, EVP_PKEY **privkey,
           STACK_OF(X509) **chain);

#endif /* RENEWALD_LOCL_H */
