#include <glite_gsplugin.h>

#include "GSOAP_H.h"
#include "CalcService.nsmap"

static const char *server = "http://localhost:19999/";

int
main(int argc, char **argv)
{	
	struct soap		soap;
	double			a, b, result;
	int				ret;


	if (argc < 4) {
		fprintf(stderr, "Usage: [add|sub] num num\n");
		exit(1);
	}

	soap_init(&soap);
	soap_set_namespaces(&soap, namespaces);
	soap_register_plugin(&soap, glite_gsplugin);

	a = strtod(argv[2], NULL);
	b = strtod(argv[3], NULL);
	switch ( *argv[1] ) {
	case 'a':
		ret = soap_call_wscalc__add(&soap, server, "", a, b, &result);
		break;
	case 's':
		ret = soap_call_wscalc__sub(&soap, server, "", a, b, &result);
		break;
	default:
		fprintf(stderr, "Unknown command\n");
		exit(2);
	}

	if ( ret ) {
		fprintf(stderr, "NECO JE V ****\n\n");
		fprintf(stderr, "plugin err: %s", glite_gsplugin_errdesc(&soap));
		soap_print_fault(&soap, stderr);
	}
	else printf("result = %g\n", result);


	return 0;
}
