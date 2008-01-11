#ident "$Header$"

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ares.h>
#include <errno.h>

#include <globus_common.h>
#include <gssapi.h>

#include "glite_gss.h"

#define tv_sub(a,b) {\
	(a).tv_usec -= (b).tv_usec;\
	(a).tv_sec -= (b).tv_sec;\
	if ((a).tv_usec < 0) {\
		(a).tv_sec--;\
		(a).tv_usec += 1000000;\
	}\
}

struct asyn_result {
	struct hostent *ent;
	int		err;
};

static int decrement_timeout(struct timeval *timeout, struct timeval before, struct timeval after)
{
        (*timeout).tv_sec = (*timeout).tv_sec - (after.tv_sec - before.tv_sec);
        (*timeout).tv_usec = (*timeout).tv_usec - (after.tv_usec - before.tv_usec);
        while ( (*timeout).tv_usec < 0) {
                (*timeout).tv_sec--;
                (*timeout).tv_usec += 1000000;
        }
        if ( ((*timeout).tv_sec < 0) || (((*timeout).tv_sec == 0) && ((*timeout).tv_usec == 0)) ) return(1);
        else return(0);
}

/* ares callback handler for ares_gethostbyname()       */
static void callback_handler(void *arg, int status, struct hostent *h) {
	struct asyn_result *arp = (struct asyn_result *) arg;

	switch (status) {
	   case ARES_SUCCESS:
		if (h && h->h_addr_list[0]) {
			arp->ent->h_addr_list =
				(char **) malloc(2 * sizeof(char *));
			if (arp->ent->h_addr_list == NULL) {
				arp->err = NETDB_INTERNAL;
				break;
			}
			arp->ent->h_addr_list[0] =
				malloc(sizeof(struct in_addr));
			if (arp->ent->h_addr_list[0] == NULL) {
				free(arp->ent->h_addr_list);
				arp->err = NETDB_INTERNAL;
				break;
			}
			memcpy(arp->ent->h_addr_list[0], h->h_addr_list[0],
				sizeof(struct in_addr));
			arp->ent->h_addr_list[1] = NULL;
			arp->err = NETDB_SUCCESS;
		} else {
			arp->err = NO_DATA;
		}
		break;
	    case ARES_EBADNAME:
	    case ARES_ENOTFOUND:
		arp->err = HOST_NOT_FOUND;
		break;
	    case ARES_ENOTIMP:
		arp->err = NO_RECOVERY;
		break;
	    case ARES_ENOMEM:
	    case ARES_EDESTRUCTION:
	    default:
		arp->err = NETDB_INTERNAL;
		break;
	}
}

static void free_hostent(struct hostent *h){
        int i;

        if (h) {
                if (h->h_name) free(h->h_name);
		if (h->h_aliases) {
			for (i=0; h->h_aliases[i]; i++) free(h->h_aliases[i]);
			free(h->h_aliases);
		}
                if (h->h_addr_list) {
                        for (i=0; h->h_addr_list[i]; i++) free(h->h_addr_list[i]);
                        free(h->h_addr_list);
                }
                free(h);
        }
}

static int asyn_gethostbyname(char **addrOut, char const *name, struct timeval *timeout) {
	struct asyn_result ar;
	ares_channel channel;
	int nfds;
	fd_set readers, writers;
	struct timeval tv, *tvp;
	struct timeval start_time,check_time;

/* start timer */
	gettimeofday(&start_time,0);

/* ares init */
	if ( ares_init(&channel) != ARES_SUCCESS ) return(NETDB_INTERNAL);
	ar.ent = (struct hostent *) calloc (sizeof(*ar.ent),1);

/* query DNS server asynchronously */
	ares_gethostbyname(channel, name, AF_INET, callback_handler,
			   (void *) &ar);

/* wait for result */
	while (1) {
		FD_ZERO(&readers);
		FD_ZERO(&writers);
		nfds = ares_fds(channel, &readers, &writers);
		if (nfds == 0)
			break;

		gettimeofday(&check_time,0);
		if (decrement_timeout(timeout, start_time, check_time)) {
			ares_destroy(channel);
			free_hostent(ar.ent);
			return(TRY_AGAIN);
		}
		start_time = check_time;

		tvp = ares_timeout(channel, timeout, &tv);

		switch ( select(nfds, &readers, &writers, NULL, tvp) ) {
			case -1: if (errno != EINTR) {
					ares_destroy(channel);
				  	free_hostent(ar.ent);
				  	return NETDB_INTERNAL;
				 } else
					continue;
			case 0: 
				FD_ZERO(&readers);
				FD_ZERO(&writers);
				/* fallthrough */
			default: ares_process(channel, &readers, &writers);
		}
	}

	ares_destroy(channel);

	if (ar.err == NETDB_SUCCESS) {
		*addrOut = malloc(sizeof(struct in_addr));
		memcpy(*addrOut,ar.ent->h_addr_list[0], sizeof(struct in_addr));
		free_hostent(ar.ent);
	}
	return(ar.err);
}

static int
do_connect(int *s, char const *hostname, int port, struct timeval *timeout)
{
   int sock;
   struct timeval before,after,to;
   struct sockaddr_in a;
   int sock_err;
   socklen_t err_len;
   char *addr;
   int h_errno;

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0) return EDG_WLL_GSS_ERROR_ERRNO;

   if (timeout) {
	     int	flags = fcntl(sock, F_GETFL, 0);
	     if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
		     return EDG_WLL_GSS_ERROR_ERRNO;
	     gettimeofday(&before,NULL);
   }
   
   if (timeout) {
      switch (h_errno = asyn_gethostbyname(&addr, hostname, timeout)) {
		case NETDB_SUCCESS:
			memset(&a,0,sizeof a);
			a.sin_family = AF_INET;
			memcpy(&a.sin_addr.s_addr,addr,sizeof a.sin_addr.s_addr);
			a.sin_port = htons(port);
			free(addr);
			break;
		case TRY_AGAIN:
			close(sock);
			return EDG_WLL_GSS_ERROR_TIMEOUT;
		case NETDB_INTERNAL: 
			/* fall through */
		default:
			close(sock);
			/* h_errno may be thread safe with Linux pthread libs,
			 * but such an assumption is not portable
			 */
			errno = h_errno;
			return EDG_WLL_GSS_ERROR_HERRNO;
      }
   } else {
      struct hostent *hp;

      hp = gethostbyname(hostname);
      if (hp == NULL) {
	 close(sock);
	 errno = h_errno;
	 return EDG_WLL_GSS_ERROR_HERRNO;
      }

      memset(&a,0,sizeof a);
      a.sin_family = AF_INET;
      memcpy(&a.sin_addr.s_addr, hp->h_addr_list[0], sizeof(a.sin_addr.s_addr));
      a.sin_port = htons(port);
   }

   if (connect(sock,(struct sockaddr *) &a,sizeof a) < 0) {
	     if (timeout && errno == EINPROGRESS) {
		     fd_set	fds;
		     FD_ZERO(&fds);
		     FD_SET(sock,&fds);
		     memcpy(&to,timeout,sizeof to);
		     gettimeofday(&before,NULL);
		     switch (select(sock+1,NULL,&fds,NULL,&to)) {
			     case -1: close(sock);
				      return EDG_WLL_GSS_ERROR_ERRNO;
			     case 0: close(sock);
				     return EDG_WLL_GSS_ERROR_TIMEOUT;
		     }
		     gettimeofday(&after,NULL);
		     tv_sub(after,before);
		     tv_sub(*timeout,after);

		     err_len = sizeof sock_err;
		     if (getsockopt(sock,SOL_SOCKET,SO_ERROR,&sock_err,&err_len)) {
			     close(sock);
			     return EDG_WLL_GSS_ERROR_ERRNO;
		     }
		     if (sock_err) {
			     close(sock);
			     errno = sock_err;
			     return EDG_WLL_GSS_ERROR_ERRNO;
		     }
	     }
	     else {
		     close(sock);
		     return EDG_WLL_GSS_ERROR_ERRNO;
	     }
   }

   *s = sock;
   return 0;
}

static int
send_token(int sock, void *token, size_t token_length, struct timeval *to)
{
   size_t num_written = 0;
   ssize_t count;
   fd_set fds;
   struct timeval timeout,before,after;
   int ret;

   if (to) {
       memcpy(&timeout,to,sizeof(timeout));
       gettimeofday(&before,NULL);
   }


   ret = 0;
   while(num_written < token_length) {
      FD_ZERO(&fds);
      FD_SET(sock,&fds);
      switch (select(sock+1, NULL, &fds, NULL, to ? &timeout : NULL)) {
	 case 0: ret = EDG_WLL_GSS_ERROR_TIMEOUT;
		 goto end;
		 break;
	 case -1: ret = EDG_WLL_GSS_ERROR_ERRNO;
		  goto end;
		  break;
      }

      count = write(sock, ((char *)token) + num_written,
	            token_length - num_written);
      if(count < 0) {
	 if(errno == EINTR)
	    continue;
	 else {
	    ret = EDG_WLL_GSS_ERROR_ERRNO;
	    goto end;
	 }
      }
      num_written += count;
   }

end:
   if (to) {
      gettimeofday(&after,NULL);
      tv_sub(after,before);
      tv_sub(*to,after);
      if (to->tv_sec < 0) {
	 to->tv_sec = 0;
	 to->tv_usec = 0;
      }
   }

   return ret;
}

static int
recv_token(int sock, void **token, size_t *token_length, struct timeval *to)
{
   ssize_t count;
   char buf[4098];
   char *t = NULL;
   char *tmp;
   size_t tl = 0;
   fd_set fds;
   struct timeval timeout,before,after;
   int ret;

   if (to) {
      memcpy(&timeout,to,sizeof(timeout));
      gettimeofday(&before,NULL);
   }

   ret = 0;
   do {
      FD_ZERO(&fds);
      FD_SET(sock,&fds);
      switch (select(sock+1, &fds, NULL, NULL, to ? &timeout : NULL)) {
	 case 0: 
	    ret = EDG_WLL_GSS_ERROR_TIMEOUT;
	    goto end;
	    break;
	 case -1:
	    ret = EDG_WLL_GSS_ERROR_ERRNO;
	    goto end;
	    break;
      }
      
      count = read(sock, buf, sizeof(buf));
      if (count < 0) {
	 if (errno == EINTR)
	    continue;
	 else {
	    ret = EDG_WLL_GSS_ERROR_ERRNO;
	    goto end;
	 }
      }

      if (count==0) {
         if (tl==0) 
            return EDG_WLL_GSS_ERROR_EOF;
         else goto end;
      }
      tmp=realloc(t, tl + count);
      if (tmp == NULL) {
	 errno = ENOMEM;
	 return EDG_WLL_GSS_ERROR_ERRNO;
      }
      t = tmp;
      memcpy(t + tl, buf, count);
      tl += count;

   } while (count < 0); /* restart on EINTR */

end:
   if (to) {
      gettimeofday(&after,NULL);
      tv_sub(after,before);
      tv_sub(*to,after);
      if (to->tv_sec < 0) {
	 to->tv_sec = 0;
	 to->tv_usec = 0;
      }
   }

   if (ret == 0) {
      *token = t;
      *token_length = tl;
   } else
      free(t);

   return ret;
}

static int
create_proxy(const char *cert_file, const char *key_file, char **proxy_file)
{
   char buf[4096];
   int in, out;
   char *name = NULL;
   int ret, len;

   *proxy_file = NULL;

   asprintf(&name, "%s/%d.lb.XXXXXX", P_tmpdir, getpid());

   out = mkstemp(name);
   if (out < 0)
      return EDG_WLL_GSS_ERROR_ERRNO;

   in = open(cert_file, O_RDONLY);
   if (in < 0) {
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      goto end;
   }
   while ((ret = read(in, buf, sizeof(buf))) > 0) {
      len = write(out, buf, ret);
      if (len != ret) {
	 ret = -1;
	 break;
      }
   }
   close(in);
   if (ret < 0) {
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      goto end;
   }

   len = write(out, "\n", 1);
   if (len != 1) {
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      goto end;
   }

   in = open(key_file, O_RDONLY);
   if (in < 0) {
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      goto end;
   }
   while ((ret = read(in, buf, sizeof(buf))) > 0) {
      len = write(out, buf, ret);
      if (len != ret) {
	 ret = -1;
	 break;
      }
   }
   close(in);
   if (ret < 0) {
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      goto end;
   }

   ret = 0;
   *proxy_file = name;

end:
   close(out);
   if (ret) {
      unlink(name);
      free(name);
   }

   return ret;
}

static int
destroy_proxy(char *proxy_file)
{
   /* XXX we should erase the contents safely (i.e. overwrite with 0's) */
   unlink(proxy_file);
   return 0;
}

int
edg_wll_gss_acquire_cred_gsi(const char *cert_file, const char *key_file, edg_wll_GssCred *cred,
      			     edg_wll_GssStatus* gss_code)
{
   OM_uint32 major_status = 0, minor_status, minor_status2;
   gss_cred_id_t gss_cred = GSS_C_NO_CREDENTIAL;
   gss_buffer_desc buffer = GSS_C_EMPTY_BUFFER;
   gss_name_t gss_name = GSS_C_NO_NAME;
   OM_uint32 lifetime;
   char *proxy_file = NULL;
   char *name = NULL;
   int ret;

   if ((cert_file == NULL && key_file != NULL) ||
       (cert_file != NULL && key_file == NULL))
      return EINVAL;

   if (cert_file == NULL) {
      major_status = gss_acquire_cred(&minor_status, GSS_C_NO_NAME, 0,
	    			      GSS_C_NO_OID_SET, GSS_C_BOTH,
				      &gss_cred, NULL, NULL);
      if (GSS_ERROR(major_status)) {
	 ret = EDG_WLL_GSS_ERROR_GSS;
	 goto end;
      }
   } else {
      proxy_file = (char *)cert_file;
      if (strcmp(cert_file, key_file) != 0 &&
	  (ret = create_proxy(cert_file, key_file, &proxy_file))) {
	 proxy_file = NULL;
	 goto end;
      }
      
      asprintf((char**)&buffer.value, "X509_USER_PROXY=%s", proxy_file);
      if (buffer.value == NULL) {
	 errno = ENOMEM;
	 ret = EDG_WLL_GSS_ERROR_ERRNO;
	 goto end;
      }
      buffer.length = strlen(proxy_file);

      major_status = gss_import_cred(&minor_status, &gss_cred, GSS_C_NO_OID, 1,
				     &buffer, 0, NULL);
      free(buffer.value);
      if (GSS_ERROR(major_status)) {
	 ret = EDG_WLL_GSS_ERROR_GSS;
	 goto end;
      }
   }

  /* gss_import_cred() doesn't check validity of credential loaded, so let's
    * verify it now */
    major_status = gss_inquire_cred(&minor_status, gss_cred, &gss_name,
	  			    &lifetime, NULL, NULL);
    if (GSS_ERROR(major_status)) {
       ret = EDG_WLL_GSS_ERROR_GSS;
       goto end;
    }

    /* Must cast to time_t since OM_uint32 is unsinged and hence we couldn't
     * detect negative values. */
    if ((time_t) lifetime <= 0) {
       major_status = GSS_S_CREDENTIALS_EXPIRED;
       minor_status = 0; /* XXX */
       ret = EDG_WLL_GSS_ERROR_GSS;
       goto end;
    }

   major_status = gss_display_name(&minor_status, gss_name, &buffer, NULL);
   if (GSS_ERROR(major_status)) {
      ret = EDG_WLL_GSS_ERROR_GSS;
      goto end;
   }
   name = buffer.value;
   memset(&buffer, 0, sizeof(buffer));
    
   *cred = calloc(1, sizeof(**cred));
   if (*cred == NULL) {
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      free(name);
      goto end;
   }

   (*cred)->gss_cred = gss_cred;
   gss_cred = GSS_C_NO_CREDENTIAL;
   (*cred)->lifetime = lifetime;
   (*cred)->name = name;

   ret = 0;

end:
   if (cert_file && key_file && proxy_file && strcmp(cert_file, key_file) != 0) {
      destroy_proxy(proxy_file);
      free(proxy_file);
   }

   if (gss_name != GSS_C_NO_NAME)
      gss_release_name(&minor_status2, &gss_name);

   if (gss_cred != GSS_C_NO_CREDENTIAL)
      gss_release_cred(&minor_status2, &gss_cred);

   if (GSS_ERROR(major_status)) {
      if (gss_code) {
	 gss_code->major_status = major_status;
	 gss_code->minor_status = minor_status;
      }
      ret = EDG_WLL_GSS_ERROR_GSS;
   }

   return ret;
}

/* XXX XXX This is black magic. "Sometimes" server refuses the client with SSL
 *  * alert "certificate expired" even if it is not true. In this case the server
 *   * slave terminates (which helps, usually), and we can reconnect transparently.
 *    */

/* This string appears in the error message in this case */
#define _EXPIRED_ALERT_MESSAGE "function SSL3_READ_BYTES: sslv3 alert certificate expired"
#define _EXPIRED_ALERT_RETRY_COUNT 10   /* default number of slaves, hope that not all
					                                              are in the bad state */
#define _EXPIRED_ALERT_RETRY_DELAY 10   /* ms */

/* XXX XXX This is black magic. "Sometimes" server refuses the client with SSL
 *  * alert "certificate expired" even if it is not true. In this case the server
 *   * slave terminates (which helps, usually), and we can reconnect transparently.
 *    */

/* This string appears in the error message in this case */
#define _EXPIRED_ALERT_MESSAGE "function SSL3_READ_BYTES: sslv3 alert certificate expired"
#define _EXPIRED_ALERT_RETRY_COUNT 10   /* default number of slaves, hope that not all
					                                              are in the bad state */
#define _EXPIRED_ALERT_RETRY_DELAY 10   /* ms */

int 
edg_wll_gss_connect(edg_wll_GssCred cred, char const *hostname, int port,
		    struct timeval *timeout, edg_wll_GssConnection *connection,
		    edg_wll_GssStatus* gss_code)
{
   int sock, ret;
   OM_uint32 maj_stat, min_stat, min_stat2, req_flags;
   int context_established = 0;
   gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
   gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
   gss_name_t server = GSS_C_NO_NAME;
   gss_ctx_id_t context = GSS_C_NO_CONTEXT;
   char *servername = NULL;
   int retry = _EXPIRED_ALERT_RETRY_COUNT;

   maj_stat = min_stat = min_stat2 = req_flags = 0;
   memset(connection, 0, sizeof(*connection));

   /* GSI specific */
   req_flags = GSS_C_GLOBUS_SSL_COMPATIBLE;

   ret = do_connect(&sock, hostname, port, timeout);
   if (ret)
      return ret;

   /* XXX find appropriate fqdn */
   asprintf (&servername, "host@%s", hostname);
   if (servername == NULL) {
      errno = ENOMEM;
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      goto end;
   }
   input_token.value = servername;
   input_token.length = strlen(servername) + 1;

   maj_stat = gss_import_name(&min_stat, &input_token,
	 		      GSS_C_NT_HOSTBASED_SERVICE, &server);
   if (GSS_ERROR(maj_stat)) {
      ret = EDG_WLL_GSS_ERROR_GSS;
      goto end;
   }

   free(servername);
   memset(&input_token, 0, sizeof(input_token));

   /* XXX if cred == GSS_C_NO_CREDENTIAL set the ANONYMOUS flag */

   do { /* XXX: the black magic above */

   do { /* XXX: the black magic above */

   /* XXX prepsat na do {} while (maj_stat == CONT) a osetrit chyby*/
   while (!context_established) {
      /* XXX verify ret_flags match what was requested */
      maj_stat = gss_init_sec_context(&min_stat, cred->gss_cred, &context,
				      GSS_C_NO_NAME, GSS_C_NO_OID,
				      req_flags | GSS_C_MUTUAL_FLAG,
				      0, GSS_C_NO_CHANNEL_BINDINGS,
				      &input_token, NULL, &output_token,
				      NULL, NULL);
      if (input_token.length > 0) {
	 free(input_token.value);
	 input_token.length = 0;
      }

      if (output_token.length != 0) {
	 ret = send_token(sock, output_token.value, output_token.length, timeout);
	 gss_release_buffer(&min_stat2, &output_token);
	 if (ret)
	    goto end;
      }

      if (GSS_ERROR(maj_stat)) {
	 if (context != GSS_C_NO_CONTEXT) {
	    gss_delete_sec_context(&min_stat2, &context, &output_token);
	    context = GSS_C_NO_CONTEXT;
      	    if (output_token.length) {
		send_token(sock, output_token.value, output_token.length, timeout);
	 	gss_release_buffer(&min_stat2, &output_token);
      	    }
	 }
	 ret = EDG_WLL_GSS_ERROR_GSS;
	 goto end;
      }

      if(maj_stat & GSS_S_CONTINUE_NEEDED) {
	 ret = recv_token(sock, &input_token.value, &input_token.length, timeout);
	 if (ret)
	    goto end;
      } else
	 context_established = 1;
   }

   /* XXX check ret_flags matches to what was requested */

   /* retry on false "certificate expired" */
   if (ret == EDG_WLL_GSS_ERROR_GSS) {
	   edg_wll_GssStatus	gss_stat;
	   char	*msg = NULL;

	   gss_stat.major_status = maj_stat;
	   gss_stat.minor_status = min_stat;
	   edg_wll_gss_get_error(&gss_stat,"",&msg);

	   if (strstr(msg,_EXPIRED_ALERT_MESSAGE)) {
		   usleep(_EXPIRED_ALERT_RETRY_DELAY);
		   retry--;
	   }
	   else retry = 0;

	   free(msg);
   }
   else retry = 0;

   } while (retry);

   /* retry on false "certificate expired" */
   if (ret == EDG_WLL_GSS_ERROR_GSS) {
	   edg_wll_GssStatus	gss_stat;
	   char	*msg = NULL;

	   gss_stat.major_status = maj_stat;
	   gss_stat.minor_status = min_stat;
	   edg_wll_gss_get_error(&gss_stat,"",&msg);

	   if (strstr(msg,_EXPIRED_ALERT_MESSAGE)) {
		   usleep(_EXPIRED_ALERT_RETRY_DELAY);
		   retry--;
	   }
	   else retry = 0;

	   free(msg);
   }
   else retry = 0;

   } while (retry);

   connection->sock = sock;
   connection->context = context;
   servername = NULL;
   ret = 0;

end:
   if (ret == EDG_WLL_GSS_ERROR_GSS && gss_code) {
      gss_code->major_status = maj_stat;
      gss_code->minor_status = min_stat;
   }
   if (server != GSS_C_NO_NAME)
      gss_release_name(&min_stat2, &server);
   if (servername == NULL)
      free(servername);
   if (ret)
      close(sock);

   return ret;
}

int
edg_wll_gss_accept(edg_wll_GssCred cred, int sock, struct timeval *timeout,
		   edg_wll_GssConnection *connection, edg_wll_GssStatus* gss_code)
{
   OM_uint32 maj_stat, min_stat, min_stat2;
   OM_uint32 ret_flags = 0;
   gss_buffer_desc input_token = GSS_C_EMPTY_BUFFER;
   gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
   gss_name_t client_name = GSS_C_NO_NAME;
   gss_ctx_id_t context = GSS_C_NO_CONTEXT;
   int ret;

   maj_stat = min_stat = min_stat2 = 0;
   memset(connection, 0, sizeof(*connection));

   /* GSI specific */
   ret_flags = GSS_C_GLOBUS_SSL_COMPATIBLE;

   do {
      ret = recv_token(sock, &input_token.value, &input_token.length, timeout);
      if (ret)
	 goto end;

      maj_stat = gss_accept_sec_context(&min_stat, &context,
	    			        cred->gss_cred, &input_token,
					GSS_C_NO_CHANNEL_BINDINGS,
					&client_name, NULL, &output_token,
					&ret_flags, NULL, NULL);
      if (input_token.length > 0) {
	 free(input_token.value);
	 input_token.length = 0;
      }

      if (output_token.length) {
	 ret = send_token(sock, output_token.value, output_token.length, timeout);
	 gss_release_buffer(&min_stat2, &output_token);
	 if (ret)
	    goto end;
      }
   } while(maj_stat & GSS_S_CONTINUE_NEEDED);

   if (GSS_ERROR(maj_stat)) {
      if (context != GSS_C_NO_CONTEXT) {
	 gss_delete_sec_context(&min_stat2, &context, &output_token);
	 context = GSS_C_NO_CONTEXT;
      	 if (output_token.length) {
		send_token(sock, output_token.value, output_token.length, timeout);
	 	gss_release_buffer(&min_stat2, &output_token);
      	 }
      }
      ret = EDG_WLL_GSS_ERROR_GSS;
      goto end;
   }

#if 0
   maj_stat = gss_display_name(&min_stat, client_name, &output_token, NULL);
   gss_release_buffer(&min_stat2, &output_token);
   if (GSS_ERROR(maj_stat)) {
      /* XXX close context ??? */
      ret = EDG_WLL_GSS_ERROR_GSS;
      goto end;
   }
#endif

   connection->sock = sock;
   connection->context = context;
   ret = 0;

end:
   if (ret == EDG_WLL_GSS_ERROR_GSS && gss_code) {
      gss_code->major_status = maj_stat;
      gss_code->minor_status = min_stat;
   }
   if (client_name != GSS_C_NO_NAME)
      gss_release_name(&min_stat2, &client_name);

   return ret;
}

int
edg_wll_gss_write(edg_wll_GssConnection *connection, const void *buf, size_t bufsize,
		  struct timeval *timeout, edg_wll_GssStatus* gss_code)
{
   OM_uint32 maj_stat, min_stat;
   gss_buffer_desc  input_token;
   gss_buffer_desc  output_token;
   int  ret;

   input_token.value = (void*)buf;
   input_token.length = bufsize;

   maj_stat = gss_wrap (&min_stat, connection->context, 1, GSS_C_QOP_DEFAULT,
			&input_token, NULL, &output_token);
   if (GSS_ERROR(maj_stat)) {
      if (gss_code) {
	 gss_code->minor_status = min_stat;
	 gss_code->major_status = maj_stat;
      }

      return EDG_WLL_GSS_ERROR_GSS;
   }

   ret = send_token(connection->sock, output_token.value, output_token.length,
	            timeout);
   gss_release_buffer(&min_stat, &output_token);

   return ret;
}


int
edg_wll_gss_read(edg_wll_GssConnection *connection, void *buf, size_t bufsize,
		 struct timeval *timeout, edg_wll_GssStatus* gss_code)
{
   OM_uint32 maj_stat, min_stat, min_stat2;
   gss_buffer_desc input_token;
   gss_buffer_desc output_token;
   size_t i, len;
   int ret;

   if (connection->bufsize > 0) {
      len = (connection->bufsize < bufsize) ? connection->bufsize : bufsize;
      memcpy(buf, connection->buffer, len);
      if (connection->bufsize - len == 0) {
	 free(connection->buffer);
	 connection->buffer = NULL;
      } else {
	 for (i = 0; i < connection->bufsize - len; i++)
	    connection->buffer[i] = connection->buffer[i+len];
      }
      connection->bufsize -= len;

      return len;
   }

   do {
      ret = recv_token(connection->sock, &input_token.value, &input_token.length,
	               timeout);
      if (ret)
	 return ret;

      ERR_clear_error();
      maj_stat = gss_unwrap(&min_stat, connection->context, &input_token,
	  		    &output_token, NULL, NULL);
      gss_release_buffer(&min_stat2, &input_token);
      if (GSS_ERROR(maj_stat)) {
	 if (gss_code) {
           gss_code->minor_status = min_stat;
           gss_code->major_status = maj_stat;
         }
	 return EDG_WLL_GSS_ERROR_GSS;
      }
   } while (maj_stat == 0 && output_token.length == 0 && output_token.value == NULL);

   if (output_token.length > bufsize) {
      connection->bufsize = output_token.length - bufsize;
      connection->buffer = malloc(connection->bufsize);
      if (connection->buffer == NULL) {
	 connection->bufsize = 0;
	 ret = EDG_WLL_GSS_ERROR_ERRNO;
	 goto end;
      }
      memcpy(connection->buffer, output_token.value + bufsize, connection->bufsize);
      output_token.length = bufsize;
   }

   memcpy(buf, output_token.value, output_token.length);
   ret = output_token.length;

end:
   gss_release_buffer(&min_stat, &output_token);

   return ret;
}

int
edg_wll_gss_read_full(edg_wll_GssConnection *connection, void *buf, 
       		      size_t bufsize, struct timeval *timeout, size_t *total,
		      edg_wll_GssStatus* gss_code)
{
   size_t	len, i;
   *total = 0;

   if (connection->bufsize > 0) {
      len = (connection->bufsize < bufsize) ? connection->bufsize : bufsize;
      memcpy(buf, connection->buffer, len);
      if (connection->bufsize - len == 0) {
	 free(connection->buffer);
	 connection->buffer = NULL;
      } else {
         for (i = 0; i < connection->bufsize - len; i++)
            connection->buffer[i] = connection->buffer[i+len];
      }
      connection->bufsize -= len;
      *total = len;
   }

   while (*total < bufsize) {
      int len;

      len = edg_wll_gss_read(connection, buf+*total, bufsize-*total,
	                     timeout, gss_code);
      if (len < 0) return len;
      *total += len;
   }

   return 0;
}

int
edg_wll_gss_write_full(edg_wll_GssConnection *connection, const void *buf,
                       size_t bufsize, struct timeval *timeout, size_t *total,
		       edg_wll_GssStatus* gss_code)
{
   return edg_wll_gss_write(connection, buf, bufsize, timeout, gss_code);
}

/* Request credential reload each 60 seconds in order to work around
 * Globus bug (not reloading expired CRLs)
 */
#define GSS_CRED_WATCH_LIMIT  60
int
edg_wll_gss_watch_creds(const char *proxy_file, time_t *last_time)
{
      struct stat     pstat;
      time_t  now;

      now = time(NULL);

      if ( now >= (*last_time+GSS_CRED_WATCH_LIMIT) ) {
              *last_time = now;
              return 1;
      }

      if (!proxy_file) return 0;
      if (stat(proxy_file,&pstat)) return -1;

      if ( pstat.st_mtime >= *last_time ) {
              *last_time = now + 1;
              return 1;
      }

      return 0;
}

int
edg_wll_gss_close(edg_wll_GssConnection *con, struct timeval *timeout)
{
   OM_uint32 min_stat;
   gss_buffer_desc output_token = GSS_C_EMPTY_BUFFER;
   struct timeval def_timeout = { 0, 100000};

   if (con->context != GSS_C_NO_CONTEXT) {
      gss_delete_sec_context(&min_stat, (gss_ctx_id_t *)&con->context, &output_token);

#if 0
      /* XXX: commented out till timeout handling in caller is fixed */

      /* send the buffer (if any) to the peer. GSSAPI specs doesn't
       * recommend sending it, but we want SSL compatibility */
      if (output_token.length && con->sock>=0) {
              send_token(con->sock, output_token.value, output_token.length,
                              timeout ? timeout : &def_timeout);
      }
#endif
      gss_release_buffer(&min_stat, &output_token);

      /* XXX can socket be open even if context == GSS_C_NO_CONTEXT) ? */
      /* XXX ensure that edg_wll_GssConnection is created with sock set to -1 */
      if (con->sock >= 0)
	 close(con->sock);
   }
   if (con->buffer)
      free(con->buffer);
   memset(con, 0, sizeof(*con));
   con->context = GSS_C_NO_CONTEXT;
   con->sock = -1;
   return 0;
}

int
edg_wll_gss_get_error(edg_wll_GssStatus *gss_err, const char *prefix, char **msg)
{
   OM_uint32 maj_stat, min_stat;
   OM_uint32 msg_ctx = 0;
   gss_buffer_desc maj_status_string = GSS_C_EMPTY_BUFFER;
   gss_buffer_desc min_status_string = GSS_C_EMPTY_BUFFER;
   char *str = NULL;
   char *line, *tmp;

   str = strdup(prefix);
   do {
      maj_stat = gss_display_status(&min_stat, gss_err->major_status,
      				    GSS_C_GSS_CODE, GSS_C_NO_OID,
				    &msg_ctx, &maj_status_string);
      if (GSS_ERROR(maj_stat))
	 break;
      maj_stat = gss_display_status(&min_stat, gss_err->minor_status,
	    			    GSS_C_MECH_CODE, GSS_C_NULL_OID,
				    &msg_ctx, &min_status_string);
      if (GSS_ERROR(maj_stat)) {
	 gss_release_buffer(&min_stat, &maj_status_string);
	 break;
      }

      asprintf(&line, ": %s (%s)", (char *)maj_status_string.value,
	       (char *)min_status_string.value);
      gss_release_buffer(&min_stat, &maj_status_string);
      gss_release_buffer(&min_stat, &min_status_string);

      tmp = realloc(str, strlen(str) + strlen(line) + 1);
      if (tmp == NULL) {
         /* abort() ? */
	 free(line);
	 free(str);
	 str = "WARNING: Not enough memory to produce error message";
	 break;
      }
      str = tmp;
      strcat(str, line);
      free(line);
   } while (!GSS_ERROR(maj_stat) && msg_ctx != 0);

   *msg = str;
   return 0;
}

int
edg_wll_gss_oid_equal(const gss_OID a, const gss_OID b)
{
   if (a == b)
      return 1;
   else {
      if (a == GSS_C_NO_OID || b == GSS_C_NO_OID || a->length != b->length)
	 return 0;
      else
	 return (memcmp(a->elements, b->elements, a->length) == 0);
   }
}

int 
edg_wll_gss_reject(int sock)
{
   /* XXX is it possible to cut & paste edg_wll_ssl_reject() ? */
   return 0;
}


int
edg_wll_gss_initialize(void)
{
   int ret;

   ret = globus_module_activate(GLOBUS_GSI_GSSAPI_MODULE);
   if (ret != GLOBUS_SUCCESS) {
      errno = EINVAL;
      ret = EDG_WLL_GSS_ERROR_ERRNO;
   }
   return ret;
}

int
edg_wll_gss_release_cred(edg_wll_GssCred *cred, edg_wll_GssStatus* gss_code)
{
   OM_uint32 maj_stat, min_stat;
   int ret = 0;

   if (gss_code)
      gss_code->major_status = gss_code->minor_status = 0;

   if (cred == NULL || *cred == NULL)
      return ret;

   if ((*cred)->gss_cred) {
      maj_stat = gss_release_cred(&min_stat, (gss_cred_id_t*)&(*cred)->gss_cred); 
      if (GSS_ERROR(maj_stat)) {
         ret = EDG_WLL_GSS_ERROR_GSS;
         if (gss_code) {
            gss_code->major_status = maj_stat;
            gss_code->minor_status = min_stat;
         }
      }
   }

   if ((*cred)->name)
      free((*cred)->name);

   free(*cred);
   *cred = NULL;

   return ret;
}

int
edg_wll_gss_get_client_conn(edg_wll_GssConnection *connection,
                            edg_wll_GssPrincipal *principal,
			    edg_wll_GssStatus* gss_code)
{
   gss_buffer_desc token = GSS_C_EMPTY_BUFFER;
   OM_uint32 maj_stat, min_stat, ctx_flags;
   gss_name_t client_name = GSS_C_NO_NAME;
   edg_wll_GssPrincipal p;
   int ret;

   maj_stat = gss_inquire_context(&min_stat, connection->context, &client_name,
                                  NULL, NULL, NULL, &ctx_flags, NULL, NULL);
   if (GSS_ERROR(maj_stat))
      goto end;

   maj_stat = gss_display_name(&min_stat, client_name, &token, NULL);
   if (GSS_ERROR(maj_stat))
      goto end;

   p = calloc(1, sizeof(*p));
   if (p == NULL) {
      errno = ENOMEM;
      ret = EDG_WLL_GSS_ERROR_ERRNO;
      goto end;
   }

   p->name = strdup(token.value);
   p->flags = ctx_flags;

   *principal = p;
   ret = 0;

end:
   if (GSS_ERROR(maj_stat)) {
      ret = EDG_WLL_GSS_ERROR_GSS;
      if (gss_code) {
         gss_code->major_status = maj_stat;
         gss_code->minor_status = min_stat;
      }
   }
	
   if (token.length)
      gss_release_buffer(&min_stat, &token);
   if (client_name != GSS_C_NO_NAME)
      gss_release_name(&min_stat, &client_name);

   return ret;
}

void
edg_wll_gss_free_princ(edg_wll_GssPrincipal principal)
{
   if (principal == NULL)
      return;

   if (principal->name)
      free(principal->name);

   free(principal);
}

int
edg_wll_gss_gethostname(char *name, int len)
{
   int ret;

   ret = globus_module_activate(GLOBUS_COMMON_MODULE);
   if (ret != GLOBUS_SUCCESS) {
      ret = gethostname(name, len);
      return ret;
   }
   ret = globus_libc_gethostname(name, len);
   globus_module_deactivate(GLOBUS_COMMON_MODULE);

   return ret;
}

char *
edg_wll_gss_normalize_subj(char *in, int replace_in)
{
	char *new, *ptr;
	size_t len;

	if (in == NULL) return NULL;
	if (replace_in) 
		new = in;
	else
		new = strdup(in);
	
	while ((ptr = strstr(new, "/emailAddress="))) {
		memcpy(ptr, "/Email=",7);
		memmove(ptr+7, ptr+14, strlen(ptr+14)+1);
	}
	
	len = strlen(new);
	while (len > 9 && !strcmp(new+len-9, "/CN=proxy")) {
		*(new+len-9) = '\0';
		len -= 9;
	}

	return new;
}

int
edg_wll_gss_equal_subj(const char *a, const char *b)
{
	char *an,*bn;
	int res;

	an = edg_wll_gss_normalize_subj((char*)a, 0);
	bn = edg_wll_gss_normalize_subj((char*)b, 0);

	if (!an || !bn)
		res = 0;
	else 
		res = !strcmp(an,bn);
	
	free(an); free(bn);
	return res;
}