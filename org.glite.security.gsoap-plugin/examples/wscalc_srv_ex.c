#include <stdio.h>
#include <getopt.h>
#include <stdsoap2.h>
#include <glite_gsplugin.h>

#include "GSOAP_H.h"
#include "CalcService.nsmap"


static struct option long_options[] = {
	{ "cert",    required_argument,      NULL,   'c' },
	{ "key",     required_argument,      NULL,   'k' },
	{ NULL, 0, NULL, 0 }
};

void
usage(const char *me)
{
	fprintf(stderr,
			"usage: %s [option]\n"
			"\t-c, --cred\t certificate file\n"
			"\t-k, --key\t private key file\n", me);
}


int
main(int argc, char **argv)
{
	struct soap				soap;
	glite_gsplugin_Context	ctx = NULL;
	char				   *name;
	char				   *cert, *key;
	int						opt;


	cert = key = NULL;
	name = strrchr(argv[0],'/');
	if ( name ) name++; else name = argv[0];

	while ((opt = getopt_long(argc, argv, "c:k:", long_options, NULL)) != EOF) {
		switch (opt) {
		case 'c': cert = optarg; break;
		case 'k': key = optarg; break;
		case '?':
		default : usage(name); exit(1);
		}
	}

	if ( cert || key ) {
		if ( glite_gsplugin_init_context(&ctx) ) { perror("init context"); exit(1); }
		ctx->cert_filename = strdup(cert? : key);
		ctx->key_filename = strdup(key? : cert);
	}

	soap_init(&soap);

	if ( soap_register_plugin_arg(&soap, glite_gsplugin, ctx? : NULL) ) {
		fprintf(stderr, "Can't register plugin\n");
		exit(1);
	}

	if ( soap_bind(&soap, NULL, 19999, 100) < 0 ) {
		soap_print_fault(&soap, stderr);
		exit(1);
	}

	while ( 1 ) {
		printf("accepting connection\n");
		if ( soap_accept(&soap) < 0 ) {
			fprintf(stderr, "soap_accept() failed!!!\n");
			soap_print_fault(&soap, stderr);
			fprintf(stderr, "plugin err: %s", glite_gsplugin_errdesc(&soap));
			break;
		}

		printf("serving connection\n");
		if ( soap_serve(&soap) ) {
			soap_print_fault(&soap, stderr);
			fprintf(stderr, "plugin err: %s", glite_gsplugin_errdesc(&soap));
		}

		soap_destroy(&soap); /* clean up class instances */
		soap_end(&soap); /* clean up everything and close socket */
	}
	soap_done(&soap); /* close master socket */

	if ( ctx ) glite_gsplugin_free_context(ctx);

	return 0;
} 

int wscalc__add(struct soap *soap, double a, double b, struct wscalc__addResponse *result)
{
	result->result = a + b;
	return SOAP_OK;
} 

int wscalc__sub(struct soap *soap, double a, double b, struct wscalc__subResponse *result)
{
	result->result = a - b;
	return SOAP_OK;
} 
