#include "renewal_locl.h"
#include "renewd_locl.h"

static const char rcsid[] = "$Header$";

#define SEPARATORS "\n"
/* GRIDMANAGER_CHECKPROXY_INTERVAL + GRIDMANAGER_MINIMUM_PROXY_TIME */
#define CONDOR_MINIMUM_PROXY_TIME (1800)

int debug = 0;
char *repository = NULL;
time_t condor_limit = CONDOR_MINIMUM_PROXY_TIME;
char *cadir = NULL;
char *vomsdir = NULL;
int voms_enabled = 0;
char *cert = NULL;
char *key = NULL;
char *vomsconf = NULL;

static volatile int die = 0, child_died = 0;

static struct option opts[] = {
   { "help",       no_argument,       NULL,  'h' },
   { "version",    no_argument,       NULL,  'v' },
   { "debug",      no_argument,       NULL,  'd' },
   { "repository", required_argument, NULL,  'r' },
   { "condor-limit", required_argument, NULL, 'c' }, 
   { "CAdir",      required_argument, NULL,  'C' },
   { "VOMSdir",    required_argument, NULL,  'V' },
   { "enable-voms", no_argument,     NULL,  'A' },
   { "voms-config", required_argument, NULL, 'G' },
   { "cert",        required_argument, NULL, 't' },
   { "key",         required_argument, NULL, 'k' },
   { NULL, 0, NULL, 0 }
};

typedef struct {
   edg_wlpr_Command code;
   void (*handler) (edg_wlpr_Request *request, edg_wlpr_Response *response);
} command_table;

static command_table commands[] = {
   { EDG_WLPR_COMMAND_REG,     register_proxy,     },
   { EDG_WLPR_COMMAND_UNREG,   unregister_proxy,   },
   { EDG_WLPR_COMMAND_GET,     get_proxy,          },
#if 0
   { EDG_WLPR_COMMAND_LIST,    list_proxies,       },
   { EDG_WLPR_COMMAND_STATUS,  status_proxy,       },
#endif
   { EDG_WLPR_COMMAND_UPDATE_DB, update_db,        },
   { 0, NULL },
};

/* static prototypes */
static void
usage(char *progname);

static int
do_listen(char *socket_name, int *sock);

static int
encode_response(edg_wlpr_Response *response, char **msg);

static command_table *
find_command(edg_wlpr_Command code);

static int
proto(int sock);

static int
doit(int sock);

static int
decode_request(const char *msg, const size_t msg_len, edg_wlpr_Request *request);

int
start_watchdog(pid_t *pid);

static void
catchsig(int sig)
{
   switch (sig) {
      case SIGINT:
      case SIGTERM:
      case SIGQUIT:
	 die = sig;
	 break;
      case SIGCHLD:
	 child_died = 1;
	 break;
      default:
	 break;
   }
}

static command_table *
find_command(edg_wlpr_Command code)
{
   command_table *c;

   for (c = commands; c->code; c++) {
      if (c->code == code)
          return c;
   }
   return NULL;
}

static int
proto(int sock)
{
   char  *buf = NULL;
   size_t  buf_len;
   int  ret;
   edg_wlpr_Response  response;
   edg_wlpr_Request  request;
   command_table  *command;

   memset(&request, 0, sizeof(request));
   memset(&response, 0, sizeof(response));

   ret = edg_wlpr_Read(sock, &buf, &buf_len);
   if (ret) {
      edg_wlpr_Log(LOG_ERR, "Error reading from client: %s",
                   edg_wlpr_GetErrorString(ret));
      return ret;
   }

   ret = decode_request(buf, buf_len, &request);
   free(buf);
   if (ret)
      goto end;

   /* XXX check request (protocol version, ...) */

   command = find_command(request.command);
   if (command == NULL) {
      ret = EDG_WLPR_ERROR_UNKNOWN_COMMAND;
      edg_wlpr_Log(LOG_ERR, "Received unknown command (%d)", request.command);
      goto end;
   }

   edg_wlpr_Log(LOG_INFO, "Received command code %d for proxy %s and jobid %s",
                request.command,
		request.proxy_filename ? request.proxy_filename : "(unspecified)",
		request.jobid ? request.jobid : "(unspecified)");

   command->handler(&request, &response);

   ret = encode_response(&response, &buf);
   if (ret)
      goto end;

   ret = edg_wlpr_Write(sock, buf, strlen(buf) + 1);
   free(buf);
   if (ret) {
      edg_wlpr_Log(LOG_ERR, "Error sending response to client: %s",
                   edg_wlpr_GetErrorString(ret));
      goto end;
   }

end:
   edg_wlpr_CleanRequest(&request);
   edg_wlpr_CleanResponse(&response);

   return ret;
}

static int
doit(int sock)
{
   int newsock;
   struct sockaddr_un client_addr;
   int client_addr_len = sizeof(client_addr);
#if 0
   next_renewal = LONG_MAX;
   size_of_proxies = PROXIES_ALLOC_SIZE;
   proxies = malloc((size_of_proxies) * sizeof(struct guarded_proxy *));
   if (proxies == NULL) {
       return ENOMEM;
   }
   proxies[0] = NULL;
#endif

   while (!die) {

      if (child_died) {
	 int pid, newpid, ret;

	 while ((pid=waitpid(-1,NULL,WNOHANG))>0)
	    ;
	 ret = start_watchdog(&newpid);
	 if (ret)
	    return ret;
	 edg_wlpr_Log(LOG_DEBUG, "Renewal slave process re-started");
	 child_died = 0;
	 continue;
      }

      newsock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
      if (newsock == -1) {
	 if (errno != EINTR)
	    edg_wlpr_Log(LOG_ERR, "accept() failed");
         continue;
      }
      edg_wlpr_Log(LOG_DEBUG, "Got connection");

      proto(newsock);

      edg_wlpr_Log(LOG_DEBUG, "Connection closed");
      close(newsock);
   }
   edg_wlpr_Log(LOG_DEBUG, "Terminating on signal %d\n",die);
   return 0;
}

static int
decode_request(const char *msg, const size_t msg_len, edg_wlpr_Request *request)
{
   char *value = NULL;
#if 0
   char *p;
   int port;
#endif
   int ret;
   int index;
   
   /* XXX add an ending zero '\0' */

   assert(msg != NULL);
   assert(request != NULL);
   
   memset(request, 0, sizeof(*request));

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_VERSION, SEPARATORS,
	 		   0, &request->version);
   if (ret) {
      edg_wlpr_Log(LOG_ERR, "Protocol error reading protocol specification: %s",
                   edg_wlpr_GetErrorString(ret));
      return ret;
   }
   
   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_COMMAND, SEPARATORS,
	 		   0, &value);
   if (ret) {
      edg_wlpr_Log(LOG_ERR, "Protocol error reading command specification: %s",
                   edg_wlpr_GetErrorString(ret));
      goto err;
   }

   ret = edg_wlpr_DecodeInt(value, (int *)(&request->command));
   if (ret) {
      edg_wlpr_Log(LOG_ERR, "Received non-numeric command specification (%s)",
                   value);
      free(value);
      goto err;
   }
   free(value);

   if (find_command(request->command) == NULL) {
      edg_wlpr_Log(LOG_ERR, "Received unknown command (%d)", request->command);
      ret = EDG_WLPR_ERROR_UNKNOWN_COMMAND;
      goto err;
   }

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_MYPROXY_SERVER,
	 		   SEPARATORS, 0, &request->myproxy_server);
   if (ret && ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND) {
      edg_wlpr_Log(LOG_ERR, "Protocol error reading myproxy server specification: %s",
                   edg_wlpr_GetErrorString(ret));
      goto err;
   }

#if 0
   request->myproxy_port = EDG_WLPR_MYPROXY_PORT; /* ??? */
   if (request->myproxy_server && (p = strchr(request->myproxy_server, ':'))) {
      *p = '\0';
      port = atol(p+1); /* XXX see myproxy for err check */
      request->myproxy_port = port;
   }
#endif

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_PROXY, SEPARATORS, 
	 		   0, &request->proxy_filename);
   if (ret && ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND) {
      edg_wlpr_Log(LOG_ERR, "Protocol error reading proxy specification: %s",
                   edg_wlpr_GetErrorString(ret));
      goto err;
   }

#if 0
   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_UNIQUE_PROXY, 
	 		   SEPARATORS, 0, &value);
   if (ret && ret != EDG_WLPR_ERROR_PARSE_NOT_FOUND)
      goto err;
   if (ret == 0 && strcasecmp(value, "yes") == 0)
      request->unique = 1;
   free(value);
#endif

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_JOBID, SEPARATORS,
	 		   0, &request->jobid);
   if (ret && ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND) {
      edg_wlpr_Log(LOG_ERR, "Protocol error reading JobId : %s",
	    	   edg_wlpr_GetErrorString(ret));
      goto err;
   }

   index = 0;
   while ((ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_ENTRY,
	       			   SEPARATORS, index, &value)) == 0) {
      char **tmp;

      tmp = realloc(request->entries, (index + 2) * sizeof(*tmp));
      if (tmp == NULL) {
	 ret = ENOMEM;
	 goto err;
      }
      request->entries = tmp;
      request->entries[index] = value;
      index++;
   }
   if (ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND)
      goto err;
   if (request->entries)
      request->entries[index] = NULL;

   return 0;

err:
   edg_wlpr_CleanRequest(request);
   return ret;
}

static int
encode_response(edg_wlpr_Response *response, char **msg)
{
   char *buf;
   size_t buf_len;
   int ret;

   buf_len = EDG_WLPR_BUF_SIZE;
   buf = malloc(buf_len);
   if (buf == NULL)
      return ENOMEM;
   buf[0] = '\0';

   ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_VERSION,
	 		     EDG_WLPR_VERSION, SEPARATORS);
   if (ret)
      goto err;

   ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_RESPONSE,
                             edg_wlpr_EncodeInt(response->response_code),
			     SEPARATORS);
   if (ret)
      goto err;

   if (response->myproxy_server) {
      char host[1024];

#if 0
      snprintf(host, sizeof(host), "%s:%d", response->myproxy_server,
               (response->myproxy_port) ? response->myproxy_port : EDG_WLPR_MYPROXY_PORT);
#endif
      ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_MYPROXY_SERVER,
                                host, SEPARATORS);
      if (ret)
         goto err;
   }

   if (response->start_time) {
      ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_START_TIME,
                                edg_wlpr_EncodeInt(response->start_time),
				SEPARATORS);
      if (ret)
         goto err;
   }

   if (response->end_time) {
      ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_END_TIME,
                                edg_wlpr_EncodeInt(response->end_time),
				SEPARATORS);
      if (ret)
         goto err;
   }

   if (response->next_renewal_time) {
      ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_RENEWAL_TIME,
                                edg_wlpr_EncodeInt(response->next_renewal_time),
				SEPARATORS);
      if (ret)
         goto err;
   }

   if (response->filenames) {
      char **p = response->filenames;
      while (*p) {
         ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_PROXY, *p,
	       			   SEPARATORS);
         if (ret)
            goto err;
         p++;
      }
   }

   buf[strlen(buf)] = '\0';
   *msg = buf;
   return 0;

err:
   free(buf);
   *msg = NULL;
   return ret;
}


static void
usage(char *progname)
{
   fprintf(stderr,"usage: %s [option]\n"
	   "\t-h, --help           display this help and exit\n"
	   "\t-v, --version        output version information and exit\n"
	   "\t-d, --debug          don't fork, print out debugging information\n"
	   "\t-r, --repository     repository directory\n"
	   "\t-c, --condor-limit   how long before expiration the proxy must be renewed\n"
	   "\t-C, --CAdir          trusted certificates directory\n"
	   "\t-V, --VOMSdir        trusted VOMS servers certificates directory\n"
	   "\t-A, --enable-voms    renew also VOMS certificates in proxies\n"
	   "\t-G, --voms-config    location of the vomses configuration file\n",
	   progname);
}

static int
do_listen(char *socket_name, int *sock)
{
   struct sockaddr_un my_addr;
   int s;
   int ret;

   assert(sock != NULL);

   memset(&my_addr, 0, sizeof(my_addr));
   my_addr.sun_family = AF_UNIX;
   strncpy(my_addr.sun_path, socket_name, sizeof(my_addr.sun_path));
   unlink(socket_name);
   umask(0177);

   s = socket(AF_UNIX, SOCK_STREAM, 0);
   if (s == -1) {
      edg_wlpr_Log(LOG_ERR, "socket(): %s", strerror(errno));
      return errno;
   }

   ret = bind(s, (struct sockaddr *)&my_addr, sizeof(my_addr));
   if (ret == -1) {
      edg_wlpr_Log(LOG_ERR, "bind(): %s", strerror(errno));
      close(s);
      return errno;
   }

   ret = listen(s, 5); /* XXX enough ? */
   if (ret == -1) {
      edg_wlpr_Log(LOG_ERR, "listen(): %s", strerror(errno));
      close(s);
      return errno;
   }

   *sock = s;
   return 0;
}

void
edg_wlpr_Log(int dbg_level, const char *format, ...)
{
   va_list ap;
   char    log_mess[1024];

   /* cannot handle the %m format argument specific for syslog() */
   va_start(ap, format);
   vsnprintf(log_mess, sizeof(log_mess), format, ap);
   va_end(ap);
   
   if (debug)
      printf("[%d] %s\n", getpid(), log_mess);
   else
      if (dbg_level < LOG_DEBUG) /* XXX make configurable */
         syslog(dbg_level, "%s", log_mess);
}

int
start_watchdog(pid_t *pid)
{
   pid_t p;

   switch ((p = fork())) {
      case -1:
	 edg_wlpr_Log(LOG_ERR, "fork() failed: %s",
	              strerror(errno));
	 return errno;
      case 0:
	 watchdog_start();
	 exit(0); 
	 break;
      default:
	 *pid = p;
	 return 0;
   }
   /* not reachable */
   exit(0);
}

int main(int argc, char *argv[])
{
   int   sock;
   char  *progname;
   int   opt;
   int   fd;
   char  sockname[PATH_MAX];
   int   ret;
   pid_t pid;
   struct sigaction	sa;

   progname = strrchr(argv[0],'/');
   if (progname) progname++; 
   else progname = argv[0];

   repository = EDG_WLPR_REPOSITORY_ROOT;
   debug = 0;

   while ((opt = getopt_long(argc, argv, "hvdr:c:C:V:AG:t:k:", opts, NULL)) != EOF)
      switch (opt) {
	 case 'h': usage(progname); exit(0);
	 case 'v': fprintf(stdout, "%s:\t%s\n", progname, rcsid); exit(0);
	 case 'd': debug = 1; break;
         case 'r': repository = optarg; break;
	 case 'c': condor_limit = atoi(optarg); break;
	 case 'C': cadir = optarg; break;
	 case 'V': vomsdir = optarg; break;
	 case 'A': voms_enabled = 1; break;
	 case 'G': vomsconf = optarg; break;
	 case 't': cert = optarg; break;
	 case 'k': key = optarg; break;
	 case '?': usage(progname); return 1;
      }

   if (optind < argc) {
      usage(progname);
      exit(1);
   }

   if (chdir(repository)) {
      edg_wlpr_Log(LOG_ERR, "Cannot access repository directory %s (%s)",
	           repository, strerror(errno));
      exit(1);
   }

   globus_module_activate(GLOBUS_GSI_CERT_UTILS_MODULE);
   globus_module_activate(GLOBUS_GSI_PROXY_MODULE);

   if (!debug)
      for (fd = 3; fd < OPEN_MAX; fd++) close(fd);

   if (!debug) {
      /* chdir ? */
      if (daemon(1,0) == -1) {
	 perror("deamon()");
	 exit(1);
      }
      openlog(progname, LOG_PID, LOG_DAEMON);
   }

   if (cert)
      setenv("X509_USER_CERT", cert, 1);

   if (key)
      setenv("X509_USER_KEY", key, 1);

   if (cadir)
      setenv("X509_CERT_DIR", cadir, 1);

   memset(&sa,0,sizeof(sa));
   sa.sa_handler = catchsig;
   sigaction(SIGINT,&sa,NULL);
   sigaction(SIGQUIT,&sa,NULL);
   sigaction(SIGTERM,&sa,NULL);
   sigaction(SIGCHLD,&sa,NULL);
   sigaction(SIGPIPE,&sa,NULL);

   ret = start_watchdog(&pid);
   if (ret)
      return 1;
  
   umask(0177);
   snprintf(sockname, sizeof(sockname), "%s%d",
	    DGPR_REG_SOCKET_NAME_ROOT, getuid());
   /* XXX check that the socket is not already active */
   ret = do_listen(sockname, &sock);
   if (ret)
      return 1;
   edg_wlpr_Log(LOG_DEBUG, "Listening at %s", sockname);

   ret = doit(sock);

   close(sock);
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
