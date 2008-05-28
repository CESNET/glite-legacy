#ifndef GLITE_SECURITY_GSCOMPAT_H
#define GLITE_SECURITY_GSCOMPAT_H

#ident "$Header: "

#ifndef GSOAP_VERSION
  #error GSOAP_VERSION required
#endif

#if GSOAP_VERSION >= 20709
  #define GLITE_SECURITY_GSOAP_CHOICE_GET(CHOICE, ITEM, TYPENAME, TYPENO) ((CHOICE)->union_##TYPENAME.ITEM)
  #define GLITE_SECURITY_GSOAP_CHOICE_SETTYPE(CHOICE, ITEM, NS, TYPENAME, TYPENO) ((CHOICE)->__union_##TYPENAME) = SOAP_UNION__##NS##__union_##TYPENAME##_##ITEM
  #define GLITE_SECURITY_GSOAP_CHOICE_ISTYPE(CHOICE, ITEM, NS, TYPENAME, TYPENO) (((CHOICE)->__union_##TYPENAME) == SOAP_UNION__##NS##__union_##TYPENAME##_##ITEM)
  /** 2.7.10 requires created dummy choice with type == 0 for empty optional choice */
  #if GSOAP_VERSION >= 20710
    #define GLITE_SECURITY_GSOAP_CHOICE_SETNULL(CHOICE, TYPENAME) ((CHOICE)->__union_##TYPENAME) = 0
  #endif
#elif GSOAP_VERSION >= 20706
  #define GLITE_SECURITY_GSOAP_CHOICE_GET(CHOICE, ITEM, TYPENAME, TYPENO) ((CHOICE)->union_##TYPENO.ITEM)
  #define GLITE_SECURITY_GSOAP_CHOICE_SETTYPE(CHOICE, ITEM, NS, TYPENAME, TYPENO) ((CHOICE)->__union_##TYPENO) = SOAP_UNION_##NS##__union_##TYPENO##_##ITEM
  #define GLITE_SECURITY_GSOAP_CHOICE_ISTYPE(CHOICE, ITEM, NS, TYPENAME, TYPENO) (((CHOICE)->__union_##TYPENO) == SOAP_UNION_##NS##__union_##TYPENO##_##ITEM)
#else
  #define GLITE_SECURITY_GSOAP_CHOICE_GET(CHOICE, ITEM, TYPENAME, TYPENO) ((CHOICE)->ITEM)
  #define GLITE_SECURITY_GSOAP_CHOICE_SETTYPE(CHOICE, ITEM, NS, TYPENAME, TYPENO)
  #define GLITE_SECURITY_GSOAP_CHOICE_ISTYPE(CHOICE, ITEM, NS, TYPENAME, TYPENO) (((CHOICE)->ITEM) != NULL)
#endif
#define GLITE_SECURITY_GSOAP_CHOICE_SET(CHOICE, ITEM, NS, TYPENAME, TYPENO, VALUE) do { \
	memset((CHOICE), 0, sizeof(*(CHOICE))); \
	GLITE_SECURITY_GSOAP_CHOICE_SETTYPE(CHOICE, ITEM, NS, TYPENAME, TYPENO); \
	GLITE_SECURITY_GSOAP_CHOICE_GET(CHOICE, ITEM, TYPENAME, TYPENO) = (VALUE); \
} while(0)

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
  #define GLITE_SECURITY_GSOAP_LIST_CREATE0(SOAP, LIST, SIZE, TYPE, N) do { \
	if ((N) != 0) (LIST) = soap_malloc((SOAP), (N) * sizeof(TYPE)); \
	else (LIST) = NULL; \
	(SIZE) = (N); \
} while (0)
  #define GLITE_SECURITY_GSOAP_LIST_DESTROY0(SOAP, LIST, SIZE) do { \
	if ((LIST) && (SIZE) != 0) soap_dealloc((SOAP), (LIST)); \
	(LIST) = NULL; \
} while (0)
  #define GLITE_SECURITY_GSOAP_LIST_GET(LIST, INDEX) (&(LIST)[INDEX])
  #define GLITE_SECURITY_GSOAP_LIST_TYPE(NS, LIST) struct NS##__##LIST *
#else
  #define GLITE_SECURITY_GSOAP_LIST_CREATE0(SOAP, LIST, SIZE, TYPE, N) do { \
	int ilist; \
	\
	if ((N) != 0) (LIST) = soap_malloc((SOAP), (N) * sizeof(void *)); \
	else (LIST) = NULL; \
	(SIZE) = (N); \
	for (ilist = 0; ilist < (N); ilist++) { \
		(LIST)[ilist] = soap_malloc((SOAP), sizeof(TYPE)); \
	} \
} while (0)
  #define GLITE_SECURITY_GSOAP_LIST_DESTROY0(SOAP, LIST, SIZE) do { \
	int ilist; \
	\
	for (ilist = 0; ilist < (SIZE); ilist++) soap_dealloc((SOAP), (LIST)[ilist]); \
	if ((LIST) && (SIZE) != 0) soap_dealloc((SOAP), (LIST)); \
	(LIST) = NULL; \
} while (0)
  #define GLITE_SECURITY_GSOAP_LIST_GET(LIST, INDEX) ((LIST)[INDEX])
  #define GLITE_SECURITY_GSOAP_LIST_TYPE(NS, LIST) struct NS##__##LIST **
#endif
#define GLITE_SECURITY_GSOAP_LIST_CREATE(SOAP, CONTAINER, LIST, TYPE, N) GLITE_SECURITY_GSOAP_LIST_CREATE0(SOAP, (CONTAINER)->LIST, (CONTAINER)->__size##LIST, TYPE, N)
#define GLITE_SECURITY_GSOAP_LIST_DESTROY(SOAP, CONTAINER, LIST) GLITE_SECURITY_GSOAP_LIST_DESTROY0(SOAP, (CONTAINER)->LIST, (CONTAINER)->__size##LIST)

#endif
