#include "renewal_locl.h"

#ident "$Header$"

/* nread() and nwrite() never return partial data */
static size_t
nread(int sock, char *buf, size_t buf_len)
{
   size_t count;
   size_t remain = buf_len;
   char *cbuf = buf;

   while (remain > 0) {
      count = read(sock, cbuf, remain);
      if (count < 0) {
	 if (errno == EINTR)
	    continue;
	 else
	    return count;
      } else
	 if (count == 0) {
	    return count;
	 }
      cbuf += count;
      remain -= count;
   }
   return buf_len;
}

static size_t
nwrite(int sock, const char *buf, size_t buf_len)
{
   const char *cbuf = buf;
   size_t count;
   size_t remain = buf_len;

   while (remain > 0) {
      count = write(sock, cbuf, remain);
      if (count < 0) {
	 if (errno == EINTR)
	    continue;
	 else
	    return count;
      }
      cbuf += count;
      remain -= count;
   }
   return buf_len;
}

int
edg_wlpr_Read(int sock, char **buf, size_t *buf_len)
{
   int ret;
   unsigned char length[4];

   ret = nread(sock, length, 4);
   if (ret == -1) {
      *buf_len = 0;
      return errno;
   }
   if (ret < 4) {
      *buf_len = 0;
      return EDG_WLPR_ERROR_UNEXPECTED_EOF; /* XXX vraci i kdyz peer spadne a zavre trubku */
   }
   *buf_len = (length[0] << 24) | 
              (length[1] << 16) | 
	      (length[2] << 8 ) | 
	      (length[3] << 0);

   *buf = malloc(*buf_len);
   if (*buf == NULL)
      return ENOMEM;

   ret = nread(sock, *buf, *buf_len);
   if (ret != *buf_len) {
      free(*buf);
      *buf_len = 0;
      return errno;
   }

   return 0;
}

int
edg_wlpr_Write(int sock, char *buf, size_t buf_len)
{
   unsigned char length[4];

   length[0] = (buf_len >> 24) & 0xFF;
   length[1] = (buf_len >> 16) & 0xFF;
   length[2] = (buf_len >> 8)  & 0xFF;
   length[3] = (buf_len >> 0)  & 0xFF;

   if (nwrite(sock, length, 4) != 4 ||
       nwrite(sock, buf, buf_len) != buf_len)
       return errno;
   
   return 0;
}

int
edg_wlpr_GetToken(const char *msg, const size_t msg_len, 
                  const char *key, const char *separators,
		  int req_index, char **value)
{
   char *p;
   size_t len;
   int index;

   assert(separators != NULL);

   /* Add ending zero ? */

   index = 0;
   p = (char *)msg;
   while (p && (p = strstr(p, key))) {
     if (index == req_index)
	break;
     index++;
     p += strlen(key);
   }
   if (p == NULL)
      return EDG_WLPR_ERROR_PROTO_PARSE_NOT_FOUND;

   p = strchr(p, '=');
   if (p == NULL)
      return EDG_WLPR_ERROR_PROTO_PARSE_ERROR;

   len = strcspn(p+1, separators);
   if (len == 0)
      return EDG_WLPR_ERROR_PROTO_PARSE_ERROR;

   *value = malloc(len + 1);
   if (*value == NULL)
      return ENOMEM;

   memcpy(*value, p+1, len);
   (*value)[len] = '\0';

   return 0;
}

int
edg_wlpr_StoreToken(char **buf, size_t *buf_len, char *command,
                    char *value, const char *separator)
{
   char line[2048];
   char *tmp;

   assert(buf != NULL);
   assert(separator != NULL);

   if (strlen(command) + 1 + strlen(value) + 2 > sizeof(line))
      return ERANGE; /* XXX */

   snprintf(line, sizeof(line), "%s%s%s", command, value, separator);

   while (strlen(*buf) + strlen(line) + 1 > *buf_len) {
      tmp = realloc(*buf, *buf_len + EDG_WLPR_BUF_SIZE);
      if (tmp == NULL)
         return ENOMEM;
      *buf = tmp;
      *buf_len += EDG_WLPR_BUF_SIZE;
   }
   strcat(*buf, line);

   return 0;
}

void
edg_wlpr_CleanRequest(edg_wlpr_Request *request)
{
   assert(request != NULL);
   if (request->version)
      free(request->version);
   if (request->proxy_filename)
      free(request->proxy_filename);
   if (request->myproxy_server)
      free(request->myproxy_server);
   if (request->jobid)
      free(request->jobid);
   if (request->entries) {
      char **p = request->entries;
      char **next;
      while (*p) {
	 next = p+1;
	 free(*p);
	 p = next;
      }
      free(request->entries);
   }

   memset(request, 0, sizeof(request));
}

void
edg_wlpr_CleanResponse(edg_wlpr_Response *response)
{
   assert(response != NULL);
   if (response->version)
      free(response->version);
   if (response->myproxy_server)
      free(response->myproxy_server);
   if (response->filenames) {
      char **p = response->filenames;
      char **next;

      while (*p) {
	 next = p+1;
	 free(*p);
	 p = next;
      }
      free(response->filenames);
   }
   memset(response, 0, sizeof(*response));
}

const char *
edg_wlpr_GetErrorString(int code)
{
   return (code == 0) ? "OK" : "Error";
}

char *
edg_wlpr_EncodeInt(int num) /* long? time */
{
   static char ret[64];

   snprintf(ret, sizeof(ret), "%d", num);
   return ret;
}

int
edg_wlpr_DecodeInt(char *str, int *num)
{
   *num = atol(str); /* XXX */
   return 0;
}
