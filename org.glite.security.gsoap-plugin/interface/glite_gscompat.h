#ifndef GLIE_SECURITY_GSCOMPAT_H
#define GLIE_SECURITY_GSCOMPAT_H

#ident "$Header: "

#ifndef GSOAP_VERSION
  #error GSOAP_VERSION required
#endif

#if GSOAP_VERSION >= 20709
  #define GLITE_SECURITY_GSOAP_CHOICE_GET(CHOICE, ITEM, TYPENAME, TYPENO) ((CHOICE)->union_##TYPENAME.ITEM)
#elif GSOAP_VERSION >= 20706
  #define GLITE_SECURITY_GSOAP_CHOICE_GET(CHOICE, ITEM, TYPENAME, TYPENO) ((CHOICE)->union_##TYPENO.ITEM)
#else
  #define GLITE_SECURITY_GSOAP_CHOICE_GET(CHOICE, ITEM, TYPENAME, TYPENO) ((CHOICE)->ITEM)
#endif

#if GSOAP_VERSION >= 20706
  #define GLITE_SECURITY_GSOAP_REASON2(SOAP) ((SOAP)->fault->SOAP_ENV__Reason ? (SOAP)->fault->SOAP_ENV__Reason->SOAP_ENV__Text : "(no reason)")
  #define GLITE_SECURITY_GSOAP_TRUE xsd__boolean__true_
  #define GLITE_SECURITY_GSOAP_FALSE xsd__boolean__false_
#else
  #define GLITE_SECURITY_GSOAP_REASON2(SOAP) ((SOAP)->fault->SOAP_ENV__Reason)
  #define GLITE_SECURITY_GSOAP_TRUE true_
  #define GLITE_SECURITY_GSOAP_FALSE false_
#endif

#define GLITE_SECURITY_GSOAP_DETAIL(SOAP) ((SOAP)->version == 2 ? (SOAP)->fault->SOAP_ENV__Detail : (SOAP)->fault->detail)
#define GLITE_SECURITY_GSOAP_REASON(SOAP) ((SOAP)->version == 2 ? GLITE_SECURITY_GSOAP_REASON2((SOAP)) : (SOAP)->fault->faultstring)

#if GSOAP_VERSION >= 20709
  #define GLITE_SECURITY_GSOAP_LIST_CREATE(SOAP, CONTAINER, LIST, TYPE, N) do { \
	(CONTAINER)->LIST = soap_malloc((SOAP), (N) * sizeof(TYPE)); \
	(CONTAINER)->__size##LIST = (N); \
} while (0);
  #define GLITE_SECURITY_GSOAP_LIST_GET(LIST, INDEX) (&(LIST)[INDEX])
#else
  #define GLITE_SECURITY_GSOAP_LIST_CREATE(SOAP, CONTAINER, LIST, TYPE, N) do { \
	int i##LIST; \
	\
	(CONTAINER)->LIST = soap_malloc((SOAP), (N) * sizeof(void *));\
	(CONTAINER)->__size##LIST = (N); \
	for (i##LIST = 0; i##LIST < (N); i##LIST++) { \
		(CONTAINER)->LIST[i##LIST] = soap_malloc((SOAP), sizeof(TYPE)); \
	} \
} while (0)
  #define GLITE_SECURITY_GSOAP_LIST_GET(LIST, INDEX) ((LIST)[INDEX])
#endif


#endif
