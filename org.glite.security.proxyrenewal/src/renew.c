#include "renewal_locl.h"
#include "renewd_locl.h"

#include "glite/security/voms/voms_apic.h"

#ident "$Header$"

#define RENEWAL_COUNTS_MAX	1000	/* the slave daemon exits after that many attemtps */

extern char *repository;
extern char *cadir;
extern char *vomsdir;
extern int voms_enabled;
extern char *vomsconf;

static int received_signal = -1, die = 0;

static void
check_renewal(char *datafile, int force_renew, int *num_renewed);

static int
renew_proxy(proxy_record *record, char *basename, char **new_proxy);

static void
register_signal(int signal);

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
   if (result)
      ret = EDG_WLPR_ERROR_GENERIC;

   return ret;
}

static void
register_signal(int signal)
{
      received_signal = signal;
      switch ((received_signal = signal)) {
	 case SIGINT:
	 case SIGTERM:
	 case SIGQUIT:
	    die = signal;
	    break;
	 default:
	    break;
      }
}

static int
renew_proxy(proxy_record *record, char *basename, char **new_proxy)
{
   char tmp_proxy[FILENAME_MAX];
   int tmp_fd;
   char repository_file[FILENAME_MAX];
   int ret = -1;
   char *p;
   char *server = NULL;
   myproxy_socket_attrs_t *socket_attrs;
   myproxy_request_t      *client_request;
   myproxy_response_t     *server_response;
   char *renewed_proxy;

   socket_attrs = malloc(sizeof(*socket_attrs));
   memset(socket_attrs, 0, sizeof(*socket_attrs));

   client_request = malloc(sizeof(*client_request));
   memset(client_request, 0, sizeof(*client_request));

   server_response = malloc(sizeof(*server_response));
   memset(server_response, 0, sizeof(*server_response));

   myproxy_set_delegation_defaults(socket_attrs, client_request);

   edg_wlpr_Log(LOG_DEBUG, "Trying to renew proxy in %s.%d",
	        basename, record->suffix);

   snprintf(tmp_proxy, sizeof(tmp_proxy), "%s.%d.myproxy.XXXXXX", 
	    basename, record->suffix);
   tmp_fd = mkstemp(tmp_proxy);
   if (tmp_fd == -1) {
      edg_wlpr_Log(LOG_ERR, "Cannot create temporary file (%s)",
                   strerror(errno));
      return errno;
   }

   snprintf(repository_file, sizeof(repository_file),"%s.%d",
	    basename, record->suffix);

   ret = get_proxy_base_name(repository_file, &client_request->username);
   if (ret)
      goto end;

   client_request->proxy_lifetime = 60 * 60 * DGPR_RETRIEVE_DEFAULT_HOURS;
   client_request->authzcreds = repository_file;

   server = (record->myproxy_server) ? record->myproxy_server :
                                       socket_attrs->pshost;
   if (server == NULL) {
      edg_wlpr_Log(LOG_ERR, "No myproxy server specified");
      ret = EINVAL;
      goto end;
   }
   socket_attrs->pshost = strdup(server);

   p = strchr(socket_attrs->pshost, ':');
   if (p) {
      *p++ = '\0';
      ret = edg_wlpr_DecodeInt(p, &socket_attrs->psport);
      if (ret)
	 goto end;
   } else
      socket_attrs->psport = MYPROXY_SERVER_PORT;

   verror_clear();
   ret = myproxy_get_delegation(socket_attrs, client_request,
	                        server_response, tmp_proxy);
   if (ret == 1) {
      ret = EDG_WLPR_ERROR_MYPROXY;
      edg_wlpr_Log(LOG_ERR, "Error contacting MyProxy server for proxy %s: %s",
	           repository_file, verror_get_string());
      verror_clear();
      goto end;
   }

   renewed_proxy = tmp_proxy;

   if (voms_enabled && record->voms_exts) {
      char tmp_voms_proxy[FILENAME_MAX];
      int tmp_voms_fd;
      
      snprintf(tmp_voms_proxy, sizeof(tmp_voms_proxy), "%s.%d.voms.XXXXXX",
	       basename, record->suffix);
      tmp_voms_fd = mkstemp(tmp_voms_proxy);
      if (tmp_voms_fd == -1) {
	 edg_wlpr_Log(LOG_ERR, "Cannot create temporary file (%s)",
	              strerror(errno));
	 ret = errno;
	 goto end;
      }

      ret = renew_voms_creds(repository_file, renewed_proxy, tmp_voms_proxy);
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

static void
check_renewal(char *datafile, int force_renew, int *num_renewed)
{
   char line[1024];
   proxy_record record;
   char *p;
   int ret, i;
   time_t current_time;
   FILE *meta_fd = NULL;
   char basename[FILENAME_MAX];
   edg_wlpr_Request request;
   edg_wlpr_Response response;
   char *new_proxy = NULL;
   char *entry = NULL;
   char **tmp;
   int num = 0;

   assert(datafile != NULL);

   *num_renewed = 0;

   memset(&record, 0, sizeof(record));
   memset(basename, 0, sizeof(basename));
   memset(&request, 0, sizeof(request));
   memset(&response, 0, sizeof(response));
   
   strncpy(basename, datafile, sizeof(basename) - 1);
   p = basename + strlen(basename) - strlen(".data");
   if (strcmp(p, ".data") != 0) {
      edg_wlpr_Log(LOG_ERR, "Meta filename doesn't end with '.data'");
      return;
   }
   *p = '\0';

   request.command = EDG_WLPR_COMMAND_UPDATE_DB;
   request.proxy_filename = strdup(basename);

   meta_fd = fopen(datafile, "r");
   if (meta_fd == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot open meta file %s (%s)",
	           datafile, strerror(errno));
      return;
   }

   current_time = time(NULL);
   edg_wlpr_Log(LOG_DEBUG, "Reading metafile %s", datafile);

   while (fgets(line, sizeof(line), meta_fd) != NULL) {
      free_record(&record);
      p = strchr(line, '\n');
      if (p)
	 *p = '\0';
      ret = decode_record(line, &record);
      if (ret)
	 continue; /* XXX exit? */
      if (record.jobids.len == 0) /* no jobid registered for this proxy */
	 continue;
      if (current_time + RENEWAL_CLOCK_SKEW >= record.end_time ||
	  record.next_renewal <= current_time ||
	  force_renew) {
	 ret = EDG_WLPR_PROXY_EXPIRED;
	 if ( record.end_time + RENEWAL_CLOCK_SKEW >= current_time) {
	    /* only try renewal if the proxy hasn't already expired */
	    ret = renew_proxy(&record, basename, &new_proxy);
         }

	 /* if the proxy wasn't renewed have the daemon planned another renewal */
	 asprintf(&entry, "%d:%s", record.suffix, (ret == 0) ? new_proxy : "");
	 if (new_proxy) {
	    free(new_proxy); new_proxy = NULL;
	 }

	 tmp = realloc(request.entries, (num + 2) * sizeof(*tmp));
	 if (tmp == NULL) {
	    free_record(&record);
	    return;
	 }
	 request.entries = tmp;
	 request.entries[num] = entry;
	 request.entries[num+1] = NULL;
	 num++;
      }
   }
   free_record(&record);

   if (num > 0) {
      ret = edg_wlpr_RequestSend(&request, &response);
      if (ret != 0)
	 edg_wlpr_Log(LOG_ERR,
	              "Failed to send update request to master (%d)", ret);
      else if (response.response_code != 0)
	 edg_wlpr_Log(LOG_ERR,
	              "Master failed to update database (%d)", response.response_code);

      /* delete all tmp proxy files which may survive */
      for (i = 0; i < num; i++) {
	 p = strchr(request.entries[i], ':');
	 if (p+1)
	    unlink(p+1);
      }
   }
   fclose(meta_fd);

   edg_wlpr_CleanResponse(&response);
   edg_wlpr_CleanRequest(&request);

   *num_renewed = num;

   return;
}

int renewal(int force_renew, int *num_renewed)
{
   DIR *dir = NULL;
   struct dirent *file;
   FILE *fd;
   int num = 0;

   edg_wlpr_Log(LOG_DEBUG, "Starting renewal process");

   *num_renewed = 0;

   if (chdir(repository)) {
      edg_wlpr_Log(LOG_ERR, "Cannot access repository directory %s (%s)",
	           repository, strerror(errno));
      return errno;
   }

   dir = opendir(repository);
   if (dir == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot open repository directory %s (%s)",
	           repository, strerror(errno));
      return errno;
   }

   while ((file = readdir(dir))) {
      /* read files of format `md5sum`.data, where md5sum() is of fixed length
	 32 chars */
      if (file->d_name == NULL || strlen(file->d_name) != 37 ||
	  strcmp(file->d_name + 32, ".data") != 0)
	 continue;
      fd = fopen(file->d_name, "r");
      if (fd == NULL) {
	 edg_wlpr_Log(LOG_ERR, "Cannot open meta file %s (%s)",
	              file->d_name, strerror(errno));
	 continue;
      }
      check_renewal(file->d_name, force_renew, &num);
      *num_renewed += num;
      fclose(fd);
   }
   closedir(dir);
   edg_wlpr_Log(LOG_DEBUG, "Finishing renewal process");
   return 0;
}

void
watchdog_start(void)
{
   struct sigaction sa;
   int force_renewal;
   int count = 0, num;
   
   memset(&sa,0,sizeof(sa));
   sa.sa_handler = register_signal;
   sigaction(SIGUSR1, &sa, NULL);
   sigaction(SIGINT,&sa,NULL);
   sigaction(SIGQUIT,&sa,NULL);
   sigaction(SIGTERM,&sa,NULL);
   sigaction(SIGPIPE,&sa,NULL);

   while (count < RENEWAL_COUNTS_MAX && !die) {
       received_signal = -1;
       sleep(60 * 5);
       force_renewal = (received_signal == SIGUSR1) ? 1 : 0;
       if (die)
	  break;
       /* XXX uninstall signal handler ? */
       renewal(force_renewal, &num);
       count += num;
   }
   edg_wlpr_Log(LOG_DEBUG, "Terminating after %d renewal attempts", count);
   exit(0);
}
