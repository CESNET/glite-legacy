#include "renewal_locl.h"
#include "renewd_locl.h"

#ifndef NOVOMS
#include <voms_apic.h>
#endif

#ident "$Header$"

#define RENEWAL_COUNTS_MAX	1000	/* the slave daemon exits after that many attemtps */

extern char *repository;
extern char *cadir;
extern char *vomsdir;
extern int voms_enabled;
extern char *vomsconf;
extern struct vomses_records vomses;

static int received_signal = -1, die = 0;

static void
check_renewal(char *datafile, int force_renew, int *num_renewed);

static int
renew_proxy(proxy_record *record, char *basename, char **new_proxy);

static void
register_signal(int signal);


static const char *
get_ssl_err()
{
   return "SSL failed";
}

int
load_proxy(const char *filename, X509 **cert, EVP_PKEY **privkey,
           STACK_OF(X509) **chain)
{
   X509 *my_cert = NULL;
   EVP_PKEY *my_key = NULL;
   STACK_OF(X509) *my_chain = NULL;
   FILE *fd = NULL;
   int ret;

   fd = fopen(filename, "r");
   if (fd == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    "Cannot read VOMS certificate (fopen() failed on %s: %s)",
	    filename, strerror(errno));
      return errno;
   }

   my_cert = PEM_read_X509(fd, NULL, NULL, NULL);
   if (my_cert == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    "Cannot read VOMS certificate (PEM_read_X509() failed: %s)",
	    get_ssl_err());
      ret = EDG_WLPR_ERROR_SSL;
      goto end;
   }

   my_key = PEM_read_PrivateKey(fd, NULL, NULL, NULL);
   if (my_key == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    "Cannot read VOMS certificate (PEM_read_PrivateKey() failed: %s)",
	    get_ssl_err());
      ret = EDG_WLPR_ERROR_SSL;
      goto end;
   }

   my_chain = sk_X509_new_null();
   if (my_chain == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot read VOMS certificate (sk_X509_new_null() failed: %s)",
		   get_ssl_err());
      ret = EDG_WLPR_ERROR_SSL;
      goto end;
   }

   while (1) {
      X509 *c;

      c = PEM_read_X509(fd, NULL, NULL, NULL);
      if (c == NULL) {
	 if (ERR_GET_REASON(ERR_peek_error()) == PEM_R_NO_START_LINE) {
	    /* End of file reached. no error */
	    ERR_clear_error();
	    break;
	 }
	 edg_wlpr_Log(LOG_ERR,
	       	      "Cannot read VOMS certificate (PEM_read_X509() failed: %s)",
		      get_ssl_err());
	 ret = EDG_WLPR_ERROR_SSL;
	 goto end;
      }
      sk_X509_push(my_chain, c);
   }

   *cert = my_cert;
   *privkey = my_key;
   *chain = my_chain;
   my_cert = NULL; my_key = NULL; my_chain = NULL;
   ret = 0;

end:
   fclose(fd);

   if (my_cert)
      X509_free(my_cert);
   if (my_key)
      EVP_PKEY_free(my_key);
   if (my_chain)
      sk_X509_pop_free(my_chain, X509_free);

   return ret;
}

static int
save_proxy(const char *filename, X509 *new_cert, EVP_PKEY *new_privkey, 
           STACK_OF(X509) *chain)
{
   FILE *fd = NULL;
   int ret, i;
   int retval = EDG_WLPR_ERROR_SSL;

   fd = fopen(filename, "w");
   if (fd == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot store proxy (fopen() failed on %s: %s)",
		   filename, strerror(errno));
      return errno;
   }

   ret = PEM_write_X509(fd, new_cert);
   if (ret == 0) {
      edg_wlpr_Log(LOG_ERR,
	           "Cannot store proxy (PEM_write_X509() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   ret = PEM_write_PrivateKey(fd, new_privkey, NULL, NULL, 0, NULL, NULL);
   if (ret == 0) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot store proxy (PEM_write_PrivateKey() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   for (i = 0; i < sk_X509_num(chain); i++) {
      X509 *cert = sk_X509_value(chain, i);
      ret = PEM_write_X509(fd, cert);
      if (ret == 0) {
	 edg_wlpr_Log(LOG_ERR,
	       	      "Cannot store proxy (PEM_write_X509() failed: %s)",
		      get_ssl_err());
	 goto end;
      }
   }
      
   retval = 0;

end:
   fclose(fd);

   return retval;
}

static int
gen_keypair(EVP_PKEY **keypair, int requested_bits)
{
   RSA *rsa = NULL;
   EVP_PKEY *key;

   *keypair = NULL;
   rsa = RSA_generate_key(requested_bits,
	 		  RSA_F4 /* public exponent */,
			  NULL, NULL);
   if (rsa == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot generate new proxy (RSA_generate_key() failed: %s)",
		   get_ssl_err());
      return EDG_WLPR_ERROR_SSL;
   }

   key = EVP_PKEY_new();
   if (key == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot generate new proxy (EVP_PKEY_new() failed: %s)",
		   get_ssl_err());
      RSA_free(rsa);
      return EDG_WLPR_ERROR_SSL;
   }

   if (EVP_PKEY_assign_RSA(key, rsa) == 0) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot generate new proxy (EVP_PKEY_assign_RSA() failed: %s)",
		   get_ssl_err());
      RSA_free(rsa);
      EVP_PKEY_free(key);
      return EDG_WLPR_ERROR_SSL;
   }

   *keypair = key;

   return 0;
}

static int
gen_subject_name(X509 *old_cert, X509 *new_cert)
{
   X509_NAME *name = NULL;
   X509_NAME_ENTRY *name_entry = NULL;
   int ret = EDG_WLPR_ERROR_SSL;

   name = X509_NAME_dup(X509_get_subject_name(old_cert));
   if (name == NULL) {
      edg_wlpr_Log(LOG_ERR,
	           "Cannot generate new proxy (X509_NAME_dup() failed: %s",
		   get_ssl_err());
      goto end;
   }

   name_entry = X509_NAME_ENTRY_create_by_NID(NULL /* make new entry */,
	 				      NID_commonName,
					      V_ASN1_APP_CHOOSE,
					      "proxy", -1);
   if (name_entry == NULL) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot generate new proxy (X509_NAME_ENTRY_create_by_NID() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   if (X509_NAME_add_entry(name, name_entry, X509_NAME_entry_count(name), 0) == 0) {
      edg_wlpr_Log(LOG_ERR,
	           "Cannot generate new proxy (X509_NAME_add_entry() failed: %s)",
		   get_ssl_err());
      goto end;
   }


   if (X509_set_subject_name(new_cert, name) == 0) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot generate new proxy (X509_set_subject_name() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   ret = 0;

end:
   if (name)
      X509_NAME_free(name);
   if (name_entry != NULL)
      X509_NAME_ENTRY_free(name_entry);

   return ret;
}

static int
create_proxy(X509 *old_cert, EVP_PKEY *old_privkey, X509_EXTENSION *extension, 
    	     X509 **new_cert, EVP_PKEY **new_privkey)
{
   /* Inspired by code from Myproxy */
   EVP_PKEY *key_pair = NULL;
   X509 *cert = NULL;
   int ret;
   int retval = EDG_WLPR_ERROR_SSL;

   ret = gen_keypair(&key_pair, 512);
   if (ret)
      return ret;

   cert = X509_new();
   if (cert == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot generate new proxy (X509_new() failed: Not enough memory)");
      goto end;
   }

   ret = gen_subject_name(old_cert, cert);
   if (ret) {
      retval = ret;
      goto end;
   }

   if (X509_set_issuer_name(cert, X509_get_subject_name(old_cert)) == 0) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot generate new proxy (X509_set_issuer_name() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   if (X509_set_serialNumber(cert, X509_get_serialNumber(old_cert)) == 0) {
      edg_wlpr_Log(LOG_ERR,
	    	   "Cannot generate new proxy (X509_set_serialNumber() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   X509_gmtime_adj(X509_get_notBefore(cert), -(60 * 5));
   X509_set_notAfter(cert, X509_get_notAfter(old_cert));

   if (X509_set_pubkey(cert, key_pair) == 0) {
      edg_wlpr_Log(LOG_ERR, 
	           "Cannot generate new proxy (X509_set_pubkey() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   /* set v3 */
   if (X509_set_version(cert, 2L) == 0) {
      edg_wlpr_Log(LOG_ERR,
	           "Cannot generate new proxy (X509_set_version() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   if (cert->cert_info->extensions != NULL)
      sk_X509_EXTENSION_pop_free(cert->cert_info->extensions,
	    		         X509_EXTENSION_free);
   cert->cert_info->extensions = sk_X509_EXTENSION_new_null();
   sk_X509_EXTENSION_push(cert->cert_info->extensions, extension);

   if (X509_sign(cert, old_privkey, EVP_md5()) == 0) {
      edg_wlpr_Log(LOG_ERR,
	           "Cannot generate new proxy (X509_sign() failed: %s)",
		   get_ssl_err());
      goto end;
   }

   *new_privkey = key_pair;
   *new_cert = cert;
   key_pair = NULL;
   cert = NULL;

   retval = 0;

end:
   if (key_pair)
      EVP_PKEY_free(key_pair);
   if (cert)
      X509_free(cert);

   return retval;
}

static int
create_voms_extension(char *buf, size_t buf_len, X509_EXTENSION **extensions)
{
   ASN1_OBJECT *voms_obj = NULL;
   ASN1_OCTET_STRING *voms_oct = NULL;

   *extensions = NULL;

   voms_oct = ASN1_OCTET_STRING_new();
   if (voms_oct == NULL) {
      edg_wlpr_Log(LOG_ERR,
	           "Cannot generate new proxy (ASN1_OCTET_STRING_new() failed: %s)",
		   get_ssl_err());
      return EDG_WLPR_ERROR_SSL;
   }

   voms_oct->data = buf;
   voms_oct->length = buf_len;

   voms_obj = OBJ_nid2obj(OBJ_txt2nid("VOMS"));
   if (voms_obj == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot generate new proxy (OBJ_nid2obj() failed");
      goto end;
   }

   *extensions = X509_EXTENSION_create_by_OBJ(NULL, voms_obj, 0, voms_oct);
   if (*extensions == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot generate new proxy (X509_EXTENSION_create_by_OBJ() failed");
      goto end;
   }

   return 0;

end:
   if (voms_oct)
      ASN1_OCTET_STRING_free(voms_oct);
   if (voms_obj)
      ASN1_OBJECT_free(voms_obj);
   return EDG_WLPR_ERROR_SSL;
}

#ifndef NOVOMS
static int
export_std_data(struct data *voms_data, char **buf)
{
   asprintf(buf, "GROUP: %s\n"
		 "ROLE:%s\n" /* the space is missing intentionaly */
		 "CAP: %s\n",
		 (voms_data->group) ? voms_data->group : "NULL",
		 (voms_data->role) ? voms_data->role : "NULL",
		 (voms_data->cap) ? voms_data->cap : "NULL");
   return 0;
}

static int
export_user_data(struct voms *voms_cert, char **buf, size_t *len)
{
   struct data **voms_data;
   char *str = NULL;
   char *ptr;

   *buf = NULL;

   switch (voms_cert->type) {
      case TYPE_NODATA:
	 *buf = strdup("NO DATA");
	 break;
      case TYPE_CUSTOM:
	 *buf = strdup(voms_cert->custom);
	 break;
      case TYPE_STD:
	 for (voms_data = voms_cert->std; voms_data && *voms_data; voms_data++) {
	    export_std_data(*voms_data, &str);
	    if (*buf == NULL)
	       ptr = calloc(strlen(str) + 1, 1);
	    else
	       ptr = realloc(*buf, strlen(*buf) + strlen(str) + 1);
	    if (ptr == NULL) {
	       return ENOMEM;
	    }
	    *buf = ptr;
	    strcat(*buf, str);
	    free(str);
	 }
	    
	 break;
      default:
	 return -1;
   }
   
   *len = strlen(*buf);
   return 0;
}

#endif

static int
encode_voms_buf(const char *label, char *data, size_t data_len,
                char **buf, size_t *buf_len)
{
   char *tmp;

   tmp = realloc(*buf, *buf_len + strlen(label) + data_len + 1);
   if (tmp == NULL)
      return ENOMEM;

   memcpy(tmp + *buf_len, label, strlen(label));

   memcpy(tmp + *buf_len + strlen(label), data, data_len);
   tmp[*buf_len + strlen(label) + data_len] = '\n';
   *buf = tmp;
   *buf_len = *buf_len + strlen(label) + data_len + 1;

   return 0;
}

static int
encode_voms_int(const char *label, int value, char **buf, size_t *buf_len)
{
   char tmp[16];

   snprintf(tmp, sizeof(tmp), "%d", value);
   return encode_voms_buf(label, tmp, strlen(tmp), buf, buf_len);
}

static int
encode_voms_str(const char *label, char *value, char **buf, size_t *buf_len)
{
   return encode_voms_buf(label, value, strlen(value), buf, buf_len);
}

#if 0
static int
VOMS_Export(struct vomsdata *voms_info, char **buf, size_t *len)
{
   struct voms *vc;
   char *enc_voms = NULL;
   size_t enc_voms_len = 0;
   char *data_buf;
   size_t data_len;
   int ret;

   if (voms_info == NULL || voms_info->data == NULL || *voms_info->data == NULL)
      return EINVAL;
   vc = *voms_info->data;

   ret = export_user_data(vc, &data_buf, &data_len);
   if (ret)
      return ret;

   encode_voms_int("SIGLEN:",   vc->siglen, &enc_voms, &enc_voms_len);
   encode_voms_buf("SIGNATURE:",vc->signature, vc->siglen,
	 	   &enc_voms, &enc_voms_len);
   enc_voms_len--; /* Signature is not followed by '\n' */
   encode_voms_str("USER:",     vc->user, &enc_voms, &enc_voms_len);
   encode_voms_str("UCA:",      vc->userca, &enc_voms, &enc_voms_len);
   encode_voms_str("SERVER:",   vc->server, &enc_voms, &enc_voms_len);
   encode_voms_str("SCA:",      vc->serverca, &enc_voms, &enc_voms_len);
   encode_voms_str("VO:",       vc->voname, &enc_voms, &enc_voms_len);
   encode_voms_str("URI:",      vc->uri, &enc_voms, &enc_voms_len);
   encode_voms_str("TIME1:",    vc->date1, &enc_voms, &enc_voms_len);
   encode_voms_str("TIME2:",    vc->date2, &enc_voms, &enc_voms_len);
   encode_voms_int("DATALEN:",  data_len, &enc_voms, &enc_voms_len);
   encode_voms_buf("",          data_buf, data_len, &enc_voms, &enc_voms_len);
   enc_voms_len--; /* the data already contains endind '\n' */

   free(data_buf);
   if (enc_voms == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot renew VOMS certificate (Not enough memory)");
      return ENOMEM;
   }
   *buf = enc_voms;
   *len = enc_voms_len;
   return 0;
}

static int
voms_cert_renew(char *hostname, int port, char *voms_subject,
      		char *proxy,
                struct voms **cur_voms_cert, struct vomsdata *voms_info)
{
   int ret = 0;
   char *command = "A";
   int err = 0;
   char *old_env_proxy = getenv("X509_USER_PROXY");

   setenv("X509_USER_PROXY", proxy, 1);

   /* hack (suggested by Vincenzo Ciaschini) to work around problem with
    * unitialized VOMS struct */
   ret = VOMS_Ordering("zzz:zzz", voms_info, &err);
   if (ret == 0) {
      edg_wlpr_Log(LOG_ERR, "Cannot renew VOMS certificate (VOMS_Ordering() failed");
      ret = EDG_WLPR_ERROR_VOMS;
      goto end;
   }

   /* XXX only attributes which are in current certificate should be requested*/
   ret = VOMS_Contact(hostname, port, (*cur_voms_cert)->server, command,
	 	      voms_info, &err);
   if (ret == 0) {
#if 0
      if (err == 1) { /* XXX cannot connect voms server */ 
	 ret = 0;
	 goto end;
      }
#endif
      edg_wlpr_Log(LOG_ERR, "Cannot renew VOMS certificate (VOMS_Contact() failed: %d)", err);
      ret = EDG_WLPR_ERROR_VOMS;
   } else
      ret = 0;

end:
   (old_env_proxy) ? setenv("X509_USER_PROXY", old_env_proxy, 1) :
      		     unsetenv("X509_USER_PROXY");

   return ret;
}

static int
renew_voms_cert(struct voms **cur_voms_cert, char *proxy, char **buf, size_t *buf_len)
{
   struct vomsdata *voms_info = NULL;
   char *hostname = NULL;
   char *p;
   int port, ret;

   hostname = strdup((*cur_voms_cert)->uri);
   p = strchr(hostname, ':');
   if (p)
      *p = '\0';
   port = (p) ? atoi(p+1) : 15000;

   voms_info = VOMS_Init(vomsdir, cadir);
   if (voms_info == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot renew VOMS certificate (VOMS_Init() failed)");
      ret = EDG_WLPR_ERROR_VOMS;
      goto end;
   }

   ret = voms_cert_renew(hostname, port, (*cur_voms_cert)->server, proxy, cur_voms_cert,
	                 voms_info);
   if (ret)
      goto end;
  
   ret = VOMS_Export(voms_info, buf, buf_len);
   if (ret) {
      edg_wlpr_Log(LOG_ERR, "Cannot renew VOMS certificate (VOMS_Export() failed)");
      ret = EDG_WLPR_ERROR_VOMS;
      goto end;
   }

   ret = 0;

end:
   if (hostname)
      free(hostname);
#if 0
   if (voms_info)
      VOMS_Destroy(voms_info);
#endif

   return ret;
}
#endif

#ifndef NOVOMS
static vomses_record *
find_vomses_record(char *hostname, int port)
{
   int i;

   for (i = 0; i < vomses.len; i++) {
      if (strcmp(vomses.val[i]->hostname, hostname) == 0 &&
	  vomses.val[i]->port == port)
	 return vomses.val[i];
   }

   return NULL;
}

static int
set_vo_params(struct voms **voms_cert, char **arg)
{
   vomses_record *r;
   char *tmp;
   int port;
   char *hostname;
   char *p;

   hostname = strdup((*voms_cert)->uri);
   p = strchr(hostname, ':');
   if (p)
      *p = '\0';
   port = (p) ? atoi(p+1) : 15000;

   r = find_vomses_record(hostname, port);
   if (r == NULL)
      return EINVAL;

   if (*arg == NULL) {
      asprintf(arg, " -voms %s", r->nick);
   } else {
      tmp = realloc(*arg, 
	            strlen(*arg) + strlen(" -voms ") + strlen(r->nick) + 1);
      if (tmp == NULL)
   	 return ENOMEM;
      *arg = tmp;
      *arg = strcat(*arg, " -voms ");
      *arg = strcat(*arg, r->nick);
   }
   return 0;
}
#endif

static int
exec_voms_proxy_init(char *arg, char *old_proxy, char *new_proxy)
{
   char command[256];
   int ret;
   char *old_env_proxy = getenv("X509_USER_PROXY");

   setenv("X509_USER_PROXY", old_proxy, 1);

   snprintf(command, sizeof(command), 
	 "edg-voms-proxy-init -out %s -key %s -cert %s -confile %s -q %s",
	 new_proxy, old_proxy, old_proxy, vomsconf, arg);
   ret = system(command);

   (old_env_proxy) ? setenv("X509_USER_PROXY", old_env_proxy, 1) :
                     unsetenv("X509_USER_PROXY");

   return ret;
}

#if 0
static int
renew_voms_certs(const char *old_proxy, const char *new_proxy)
{
   struct vomsdata *voms_info = NULL;
   struct voms **voms_cert = NULL;
   STACK_OF(X509) *chain = NULL;
   EVP_PKEY *privkey = NULL;
   X509 *cert = NULL;
   int ret, err;
   char *buf = NULL;
   size_t buf_len = 0;
   X509_EXTENSION *extension = NULL;
   X509 *new_cert = NULL;
   EVP_PKEY *new_privkey = NULL;

   voms_info = VOMS_Init(vomsdir, cadir);
   if (voms_info == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot initialize VOMS context (VOMS_Init() failed)");
      return EDG_WLPR_ERROR_VOMS;
   }

   ret = load_proxy(old_proxy, &cert, &privkey, &chain);
   if (ret)
      goto end;

   ret = VOMS_Retrieve(cert, chain, RECURSE_CHAIN, voms_info, &err);
   if (ret == 0) {
      if (err == VERR_NOEXT) {
	 /* no VOMS cred, no problem; continue */
	 ret = 0;
      } else {
         edg_wlpr_Log(LOG_ERR, "Cannot get VOMS certificate(s) from proxy");
         ret = EDG_WLPR_ERROR_VOMS;
      }
      goto end;
   }

   for (voms_cert = voms_info->data; voms_cert && *voms_cert; voms_cert++) {
      char *tmp, *ptr;
      size_t tmp_len;

      ret = renew_voms_cert(voms_cert, old_proxy, &tmp, &tmp_len);
      if (ret)
	 continue;
      ptr = realloc(buf, buf_len + tmp_len);
      if (ptr == NULL) {
	 ret = ENOMEM;
	 goto end;
      }
      buf = ptr;
      memcpy(buf + buf_len, tmp, tmp_len);
      buf_len += tmp_len;
   }

   if (buf == NULL) {
      /* no extension renewed, return */
      ret = 0;
      goto end;
   }

   ret = create_voms_extension(buf, buf_len, &extension);
   if (ret)
      goto end;

   X509_free(cert);
   EVP_PKEY_free(privkey);
   sk_X509_pop_free(chain, X509_free);

   ret = load_proxy(new_proxy, &cert, &privkey, &chain);
   if (ret)
      goto end;

   ret = create_proxy(cert, privkey, extension, &new_cert, &new_privkey);
   if (ret)
      goto end;

   sk_X509_insert(chain, cert, 0);
	 
   ret = save_proxy(new_proxy, new_cert, new_privkey, chain);
   if (ret)
      goto end;

   ret = 0;

end:
   VOMS_Destroy(voms_info);

   return ret;
}
#else /* 0 */

#ifdef NOVOMS
static int
renew_voms_certs(const char *old_proxy, char *myproxy_proxy, const char *new_proxy)
{
	return 0;
}

#else
static int
renew_voms_certs(const char *old_proxy, char *myproxy_proxy, const char *new_proxy)
{
   struct vomsdata *voms_info = NULL;
   struct voms **voms_cert = NULL;
   STACK_OF(X509) *chain = NULL;
   EVP_PKEY *privkey = NULL;
   X509 *cert = NULL;
   int ret, err;
   char *arg = NULL;

   voms_info = VOMS_Init(vomsdir, cadir);
   if (voms_info == NULL) {
      edg_wlpr_Log(LOG_ERR, "Cannot initialize VOMS context (VOMS_Init() failed)");
      return EDG_WLPR_ERROR_VOMS;
   }

   ret = load_proxy(old_proxy, &cert, &privkey, &chain);
   if (ret)
      goto end;

   ret = VOMS_Retrieve(cert, chain, RECURSE_CHAIN, voms_info, &err);
   if (ret == 0) {
      if (err == VERR_NOEXT) {
	 /* no VOMS cred, no problem; continue */
	 ret = 0;
      } else {
         edg_wlpr_Log(LOG_ERR, "Cannot get VOMS certificate(s) from proxy");
         ret = EDG_WLPR_ERROR_VOMS;
      }
      goto end;
   }

   for (voms_cert = voms_info->data; voms_cert && *voms_cert; voms_cert++) {
      ret = set_vo_params(voms_cert, &arg);
      if (ret)
	 goto end;
   }
   ret = exec_voms_proxy_init(arg, myproxy_proxy, new_proxy);

end:
   VOMS_Destroy(voms_info);
   return ret;
}
#endif /* NOVOMS */

#endif /* 0 */

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

   snprintf(tmp_proxy, sizeof(tmp_proxy), "%s.%d.renew.XXXXXX", 
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

   ret = myproxy_get_delegation(socket_attrs, client_request,
	                        server_response, tmp_proxy);
   if (ret == 1) {
      ret = EDG_WLPR_ERROR_MYPROXY;
      edg_wlpr_Log(LOG_ERR, "Cannot get renewed proxy from Myproxy server");
      goto end;
   }

   renewed_proxy = tmp_proxy;

   if (voms_enabled) {
      char tmp_voms_proxy[FILENAME_MAX];
      int tmp_voms_fd;
      
      snprintf(tmp_voms_proxy, sizeof(tmp_voms_proxy), "%s.%d.renew.XXXXXX",
	       basename, record->suffix);
      tmp_voms_fd = mkstemp(tmp_voms_proxy);
      if (tmp_voms_fd == -1) {
	 edg_wlpr_Log(LOG_ERR, "Cannot create temporary file (%s)",
	              strerror(errno));
	 ret = errno;
	 goto end;
      }

      ret = renew_voms_certs(repository_file, tmp_proxy, tmp_voms_proxy);
      if (ret)
	 goto end;

      renewed_proxy = tmp_voms_proxy;
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

   /* load_vomses(); */

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
