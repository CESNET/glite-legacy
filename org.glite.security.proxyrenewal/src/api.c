#include "renewal.h"
#include "renewal_locl.h"

#ident "$Header$"

#define SEPARATORS "\n"

/* prototypes of static routines */
static int
encode_request(edg_wlpr_Request *request, char **msg);

static int
decode_response(const char *msg, const size_t msg_len, edg_wlpr_Response *response);

static int
do_connect(char *socket_name, int *sock);

static int
send_request(int sock, edg_wlpr_Request *request, edg_wlpr_Response *response);


static int 
encode_request(edg_wlpr_Request *request, char **msg)
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

   ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_COMMAND,
	                     edg_wlpr_EncodeInt(request->command),
			     SEPARATORS);
   if (ret)
      goto err;

   if (request->myproxy_server) {
      char host[1024];

#if 0
      snprintf(host, sizeof(host), "%s:%d", request->myproxy_server, 
	       (request->myproxy_port) ? request->myproxy_port : EDG_WLPR_MYPROXY_PORT); /* XXX let server decide ? */
#else
      snprintf(host, sizeof(host), "%s", request->myproxy_server);
#endif
      ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_MYPROXY_SERVER,
	                        host, SEPARATORS);
      if (ret)
	 goto err;
   }

   if (request->proxy_filename) {
      ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_PROXY,
	                        request->proxy_filename, SEPARATORS);
      if (ret)
	 goto err;
   }

   if (request->jobid) {
      ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_JOBID,
	    			request->jobid, SEPARATORS);
      if (ret)
	 goto err;
   }

   if (request->entries) {
      char **p = request->entries;
      while (*p) {
	 ret = edg_wlpr_StoreToken(&buf, &buf_len, EDG_WLPR_PROTO_ENTRY,
	       			   *p, SEPARATORS);
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

static int
decode_response(const char *msg, const size_t msg_len, edg_wlpr_Response *response)
{
   int ret;
   char *value = NULL;
   /* char *p; */
   int i;
   int current_size = 0;

   /* XXX add an ending zero '\0' */

   assert(msg != NULL);
   assert(response != NULL);

   memset(response, 0, sizeof(*response));

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_VERSION, SEPARATORS,
	                   0, &response->version);
   if (ret)
      goto err;

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_RESPONSE, SEPARATORS,
	 		   0, &value);
   if (ret)
      goto err;

   ret = edg_wlpr_DecodeInt(value, (int *)(&response->response_code));
   free(value);
   if (ret)
      goto err;

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_MYPROXY_SERVER,
	 		   SEPARATORS, 0, &response->myproxy_server);
   if (ret && ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND)
      goto err;

#if 0
   response->myproxy_port = EDG_WLPR_MYPROXY_PORT; /* ??? */
   if (response->myproxy_server && (p = strchr(response->myproxy_server, ':'))) {
      int port;
      *p = '\0';
      port = atol(p+1); /* XXX */
      response->myproxy_port = port;
   }
#endif

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_START_TIME, SEPARATORS, 
	 		   0, &value);
   if (ret && ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND)
      goto err;
   if (ret == 0) {
      ret = edg_wlpr_DecodeInt(value, (int *)(&response->start_time));
      free(value);
      if (ret)
         goto err;
   }

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_END_TIME, SEPARATORS,
	 		   0, &value);
   if (ret && ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND)
      goto err;
   if (ret == 0) { 
      ret = edg_wlpr_DecodeInt(value, (int *)(&response->end_time));
      free(value);
      if (ret)
	 goto err;
   }

   ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_RENEWAL_TIME,
	 		   SEPARATORS, 0, &value);
   if (ret && ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND)
      goto err;
   if (ret == 0) {
      ret = edg_wlpr_DecodeInt(value, (int *)(&response->next_renewal_time));
      free(value);
      if (ret)
	 goto err;
   }

   /* XXX Counter */

   i = 0;
   while ((ret = edg_wlpr_GetToken(msg, msg_len, EDG_WLPR_PROTO_PROXY,
	       			   SEPARATORS, i, &value)) == 0) {
      if (i >= current_size) {
	 char **tmp;

	 tmp = realloc(response->filenames, 
	               (current_size + 16 + 1) * sizeof(*tmp));
	 if (tmp == NULL) {
	    ret = ENOMEM;
	    goto err;
	 }
	 response->filenames = tmp;
	 current_size += 16;
      }
      response->filenames[i] = value;
      i++;
   }
   if (ret != EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND)
      goto err;
   if (response->filenames)
      response->filenames[i] = NULL;

   return 0;

err:
   edg_wlpr_CleanResponse(response);

   return ret;
}

static int
do_connect(char *socket_name, int *sock)
{
   struct sockaddr_un my_addr;
   int s;
   int ret;

   assert(sock != NULL);
   memset(&my_addr, 0, sizeof(my_addr));

   s = socket(AF_UNIX, SOCK_STREAM, 0);
   if (s == -1) {
      return errno;
   }

   my_addr.sun_family = AF_UNIX;
   strncpy(my_addr.sun_path, socket_name, sizeof(my_addr.sun_path));

   ret = connect(s, (struct sockaddr *) &my_addr, sizeof(my_addr));
   if (ret == -1) {
      close(s);
      return errno;
   }

   *sock = s;
   return 0;
}

static int
send_request(int sock, edg_wlpr_Request *request, edg_wlpr_Response *response)
{
   int ret;
   char *buf = NULL;
   size_t buf_len;

   /* timeouts ?? */

   ret = encode_request(request, &buf);
   if (ret)
      return ret;

   ret = edg_wlpr_Write(sock, buf, strlen(buf) + 1);
   free(buf);
   if (ret)
      return ret;

   ret = edg_wlpr_Read(sock, &buf, &buf_len);
   if (ret)
      return ret;

   ret = decode_response(buf, buf_len, response);
   free(buf);
   if (ret)
      return ret;

   return 0;
}

int
edg_wlpr_RequestSend(edg_wlpr_Request *request, edg_wlpr_Response *response)
{
   char sockname[1024];
   int ret;
   int sock;

   snprintf(sockname, sizeof(sockname), "%s%d",
	    DGPR_REG_SOCKET_NAME_ROOT, getuid());
   ret = do_connect(sockname, &sock);
   if (ret)
      return ret;

   ret = send_request(sock, request, response);

   close(sock);
   return ret;
}

int
edg_wlpr_RegisterProxyExt(const char *filename, const char * server,
			  unsigned int port,
                          edg_wlc_JobId jobid, int flags,
			  char **repository_filename)
{
   edg_wlpr_Request request;
   edg_wlpr_Response response;
   int ret;

   memset(&request, 0, sizeof(request));
   memset(&response, 0, sizeof(response));

   request.command = EDG_WLPR_COMMAND_REG;
   request.myproxy_server = server;
   request.proxy_filename = filename;
   request.jobid = edg_wlc_JobIdUnparse(jobid);
   if (request.jobid == NULL)
      return EINVAL; /* XXX */

   ret = edg_wlpr_RequestSend(&request, &response);
   free(request.jobid);
   if (ret == 0 && response.response_code == 0 && repository_filename &&
       response.filenames && response.filenames[0] )
      *repository_filename = strdup(response.filenames[0]);

   if (ret == 0)
      ret = response.response_code;

   edg_wlpr_CleanResponse(&response);
   
   return ret;
}

int
edg_wlpr_RegisterProxy(const char *filename, const char *jdl,
                       int flags, char **repository_filename)
{
   char server[1024];
   size_t server_len;
   unsigned int port = 0;
   char *p, *q;
   
   memset(server, 0, sizeof(server));
   
   /* parse JDL and find information about myproxy server */
   p = strstr(jdl, JDL_MYPROXY);
   if (p == NULL)
      return 0; /* XXX */
   q = strchr(p, '\n'); /* XXX */
   if (q)
      server_len = q - p;
   else 
      server_len = jdl + strlen(jdl) - p;
   if (server_len >= sizeof(server))
      return EINVAL; /* XXX */
   strncmp(server, p, sizeof(server));

   return (edg_wlpr_RegisterProxyExt(filename, server, port, NULL, flags, 
	                             repository_filename));
}

int
edg_wlpr_UnregisterProxy(edg_wlc_JobId jobid, const char *repository_filename)
{
   edg_wlpr_Request request;
   edg_wlpr_Response response;
   int ret;

   memset(&request, 0, sizeof(request));
   memset(&response, 0, sizeof(response));

   request.command = EDG_WLPR_COMMAND_UNREG;
   request.proxy_filename = repository_filename;
   request.jobid = edg_wlc_JobIdUnparse(jobid);
   if (request.jobid == NULL)
      return EINVAL;

   ret = edg_wlpr_RequestSend(&request, &response);
   free(request.jobid);
   
   if (ret == 0)
      ret = response.response_code;
   edg_wlpr_CleanResponse(&response);

   return ret;
}

int
edg_wlpr_GetList(int *count, char **list)
{
   return ENOSYS; /* XXX */
}

int
edg_wlpr_GetStatus(const char *filename, char **info)
{
   return ENOSYS; /* XXX */
}

static const char* const errTexts[] = {
   "Unexpected EOF from peer",
   "Generic error",
   "Protocol parse error",
   "Compulsory element not found in message",
   "Unknown protocol command",
   "SSL error",
   "Error from Myproxy server",
   "Proxy not registered",
   "Proxy expired",
   "VOMS error",
};

const char *
edg_wlpr_GetErrorText(int code)
{
   return code ?
           (code <= EDG_WLPR_ERROR_BASE ?
	            strerror(code) :
		    errTexts[code - EDG_WLPR_ERROR_BASE - 1]
	   ) :
	   NULL;
}

int
edg_wlpr_GetProxy(edg_wlc_JobId jobid, char **repository_filename)
{
   edg_wlpr_Request request;
   edg_wlpr_Response response;
   int ret;

   memset(&request, 0, sizeof(request));
   memset(&response, 0, sizeof(response));

   request.command = EDG_WLPR_COMMAND_GET;
   request.jobid = edg_wlc_JobIdUnparse(jobid);
   if (request.jobid == NULL)
      return EINVAL;

   ret = edg_wlpr_RequestSend(&request, &response);
   free(request.jobid);

   if (ret == 0 && response.response_code == 0 && repository_filename &&
       response.filenames && response.filenames[0] )
      *repository_filename = strdup(response.filenames[0]);
   
   if (ret == 0)
      ret = response.response_code;
   edg_wlpr_CleanResponse(&response);

   return ret;
}
