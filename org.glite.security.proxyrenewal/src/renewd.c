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

char *vomsconf = "/opt/edg/etc/vomses";
#ifndef NOVOMS
struct vomses_records vomses;
#endif

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

   edg_wlpr_Log(LOG_INFO, "Received command code %d for proxy %s",
                request.command,
                request.proxy_filename ? request.proxy_filename : "(unspecified)");

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

#if 0
   sigemptyset(&sset);
   sigaddset(&sset,SIGTERM);
   sigaddset(&sset,SIGINT);
   sigaddset(&sset, SIGKILL);
   sigaddset(&sset, SIGUSR1);
   sigaddset(&sset, SIGALRM);
   sigprocmask(SIG_BLOCK,&sset,NULL);
#endif

   while (1) {
#if 0
      sigprocmask(SIG_UNBLOCK,&sset,NULL);
      newsock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
      sigprocmask(SIG_BLOCK,&sset,NULL);

      if (newsock == -1) {
         if (errno == EINTR) /* ERESTARTSYS */
             proxy_renewal(received_signal);
         else
            log();
         continue;
      }
#else
      newsock = accept(sock, (struct sockaddr *) &client_addr, &client_addr_len);
      if (newsock == -1) {
         edg_wlpr_Log(LOG_ERR, "accept() failed");
         continue;
      }
      edg_wlpr_Log(LOG_DEBUG, "Got connection");

#endif

      proto(newsock);

      edg_wlpr_Log(LOG_DEBUG, "Connection closed");
      close(newsock);
   }
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

#ifdef NOVOMS
static int
load_vomses()
{
	return ENOSYS;
}

#else
static int
load_vomses()
{
   FILE *fd = NULL;
   char line[1024];
   char *nick, *hostname;
   int port;
   vomses_record *rec;
   vomses_record **tmp;
   char *p;
   
   fd = fopen(vomsconf, "r");
   if (fd == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot open vomses configuration file (%s)",
	           strerror(errno));
      return errno;
   }
   while (fgets(line, sizeof(line), fd) != NULL) {
      p = line;
      if (*p != '"') {
	 edg_wlpr_Log(LOG_ERR, "Parsing error when reading vomses configuration file");
	 return EINVAL;
      }
      nick = strdup(strtok(p+1, "\""));

      p = strtok(NULL, "\"");
      hostname = strdup(strtok(NULL, "\""));

      p = strtok(NULL, "\"");
      port = atoi(strdup(strtok(NULL, "\"")));

      if (nick == NULL || hostname == NULL) {
	 edg_wlpr_Log(LOG_ERR, "Parsing error when reading vomses configuration file");
	 return EINVAL;
      }

      rec = calloc(1, sizeof(*rec));
      if (rec == NULL) {
	 edg_wlpr_Log(LOG_ERR, "Not enough memory");
	 return ENOMEM;
      }
      rec->nick = nick;
      rec->hostname = hostname;
      rec->port = port;

      tmp = realloc(vomses.val, vomses.len + 1);
      if (tmp == NULL) {
	 edg_wlpr_Log(LOG_ERR, "Not enough memory");
	 return ENOMEM;
      }
      vomses.val = tmp;
      vomses.len++;

      vomses.val[vomses.len-1] = rec;
   }
   fclose(fd);
   return 0;
}
#endif

int main(int argc, char *argv[])
{
   int   sock;
   char  *progname;
   int   opt;
   int   fd;
   char  sockname[PATH_MAX];
   int   ret;
   pid_t pid;

   progname = strrchr(argv[0],'/');
   if (progname) progname++; 
   else progname = argv[0];

   repository = EDG_WLPR_REPOSITORY_ROOT;
   debug = 0;

   while ((opt = getopt_long(argc, argv, "hvdr:c:C:V:AG:", opts, NULL)) != EOF)
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

   if (voms_enabled) {
      char *path;
      char *new_path;
      ret = load_vomses();
      if (ret)
	 return 1;
      setenv("GLOBUS_VERSION", "22", 0);
      if (VOMS_INSTALL_PATH != NULL && *VOMS_INSTALL_PATH != '\0') {
	 path = getenv("PATH");
	 asprintf(&new_path, "%s:%s/bin", path, VOMS_INSTALL_PATH);
         setenv("PATH", new_path, 1);
      }
   }
   
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

#if 0
   /* XXX ??? */
   install_handlers();
#endif
   

#if 0
   /* XXX this overrides setings done by install_handlers()? */
   signal(SIGTERM, cleanup);
   signal(SIGINT, cleanup);
   signal(SIGKILL, cleanup);
   signal(SIGPIPE, SIG_IGN);

   atexit(cleanup);
#endif

   ret = doit(sock);

   close(sock);
   return ret;
}
