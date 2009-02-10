#include <stdio.h>
#include <getopt.h>
#include <stdsoap2.h>
#include <glite_gsplugin.h>
#include <glite_gsplugin-int.h>

#include "GSOAP_H.h"
#include "CalcService.nsmap"


static struct option long_options[] = {
	{ "cert",    required_argument,      NULL,   'c' },
	{ "key",     required_argument,      NULL,   'k' },
	{ "port",    required_argument,      NULL,   'p' },
	{ NULL, 0, NULL, 0 }
};

void
usage(const char *me)
{
	fprintf(stderr,
			"usage: %s [option]\n"
			"\t-p, --port\t listening port\n"
			"\t-c, --cred\t certificate file\n"
			"\t-k, --key\t private key file\n", me);
}


int
main(int argc, char **argv)
{
	struct soap				soap;
	edg_wll_GssStatus		gss_code;
	edg_wll_GssCred			cred = NULL;
	edg_wll_GssConnection		connection;
	glite_gsplugin_Context	ctx;
	struct sockaddr_in		a;
	int						alen;
	char				   *name, *msg;
	int						opt,
							port = 19999;
	char				*cert_filename = NULL, *key_filename = NULL;
	int						sock;


	name = strrchr(argv[0],'/');
	if ( name ) name++; else name = argv[0];

	if ( glite_gsplugin_init_context(&ctx) ) { perror("init context"); exit(1); }

	while ((opt = getopt_long(argc, argv, "c:k:p:", long_options, NULL)) != EOF) {
		switch (opt) {
		case 'p': port = atoi(optarg); break;
		case 'c': cert_filename = strdup(optarg); break;
		case 'k': key_filename = strdup(optarg); break;
		case '?':
		default : usage(name); exit(1);
		}
	}

	if ( edg_wll_gss_acquire_cred_gsi(cert_filename, key_filename, &cred, &gss_code) ) {
		edg_wll_gss_get_error(&gss_code, "Failed to read credential", &msg);
		fprintf(stderr, "%s\n", msg);
		free(msg);
		exit(1);
	}
	if (cred->name) {
		printf("server running with certificate: %s\n", cred->name);
	}

	glite_gsplugin_use_credential(ctx, cred);

	soap_init(&soap);
	soap_set_namespaces(&soap, namespaces);

	if ( soap_register_plugin_arg(&soap, glite_gsplugin, ctx) ) {
		fprintf(stderr, "Can't register plugin\n");
		exit(1);
	}

	alen = sizeof(a);
	if ( (sock = socket(PF_INET,SOCK_STREAM,0)) < 0 ) { perror("socket()"); exit(1); }
	a.sin_family = AF_INET;
	a.sin_port = htons(port);
	a.sin_addr.s_addr = INADDR_ANY;
	if ( bind(sock, (struct sockaddr *)&a, sizeof(a)) ) { perror("bind()"); exit(1); }
	if ( listen(sock, 100) ) { perror("listen()"); exit(1); }

	bzero((char *) &a, alen);

	while ( 1 ) {
		int conn;

		printf("accepting connection\n");
		if ( (conn = accept(sock, (struct sockaddr *) &a, &alen)) < 0 ) {
			close(sock);
			perror("accept");
			exit(1);
		}
		if ( edg_wll_gss_accept(cred,conn,ctx->timeout,&connection,&gss_code) ){
			edg_wll_gss_get_error(&gss_code, "Failed to read credential", &msg);
			fprintf(stderr, "%s\n", msg);
			free(msg);
			exit(1);
		}

		glite_gsplugin_set_connection(ctx, &connection); 

		printf("serving connection\n");
		if ( soap_serve(&soap) ) {
			soap_print_fault(&soap, stderr);
			fprintf(stderr, "plugin err: %s", glite_gsplugin_errdesc(&soap));
		}

		soap_destroy(&soap); /* clean up class instances */
		soap_end(&soap); /* clean up everything and close socket */
		edg_wll_gss_close(&connection, NULL);
	}

	soap_done(&soap); /* close master socket */

	glite_gsplugin_free_context(ctx);
	edg_wll_gss_release_cred(&cred, NULL);

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
