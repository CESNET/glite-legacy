#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>
#include <netdb.h>
#include <limits.h>
#include <syslog.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "glite/lb/srvbones.h"
#include "glite/lb/context.h"
#include "glite/lb/context-int.h"
#ifdef LB_PERF
#include "glite/lb/lb_perftest.h"
#include "glite/lb/srv_perf.h"

enum lb_srv_perf_sink sink_mode;
#endif

extern int edg_wll_DBCheckVersion(edg_wll_Context, const char *);
extern edg_wll_ErrorCode edg_wll_Open(edg_wll_Context ctx, char *cs);
extern edg_wll_ErrorCode edg_wll_Close(edg_wll_Context);
extern int edg_wll_StoreProtoProxy(edg_wll_Context ctx);
extern int edg_wll_ServerHTTP(edg_wll_Context ctx);

extern char *lbproxy_ilog_socket_path;
extern char *lbproxy_ilog_file_prefix;


#define DEFAULTCS			"lbserver/@localhost:lbproxy"

#define CON_QUEUE		20	/* accept() */
#define SLAVE_OVERLOAD		10	/* queue items per slave */
#define IDLE_TIMEOUT		10	/* keep idle connection that many seconds */
#define REQUEST_TIMEOUT		120	/* one client may ask one slave multiple times */
#define SLAVE_CONNS_MAX		500	/* commit suicide after that many connections */

/* file to store pid and generate semaphores key
 */
#ifndef GLITE_LBPROXY_PIDFILE
#define GLITE_LBPROXY_PIDFILE		"/var/run/glite-lbproxy.pid"
#endif

#ifndef GLITE_LBPROXY_SOCK_PREFIX
#define GLITE_LBPROXY_SOCK_PREFIX	"/tmp/lb_proxy_"
#endif

#ifndef dprintf
#define dprintf(x)			{ if (debug) printf x; }
#endif

#define sizofa(a)			(sizeof(a)/sizeof((a)[0]))


int			debug  = 0;
static const int	one = 1;
static char		*dbstring = NULL;
static char		sock_store[PATH_MAX],
			sock_serve[PATH_MAX];
static int		slaves = 10,
			semaphores = -1,
			con_queue = CON_QUEUE,
			semset;
static char		host[300];
static char *		port;
int			transactions = -1;
int			use_dbcaps = 0;


static struct option opts[] = {
	{"port",		1, NULL,	'p'},
	{"con-queue",		1, NULL,	'c'},
	{"debug",		0, NULL,	'd'},
	{"silent",		0, NULL,	'z'},
	{"mysql",		1, NULL,	'm'},
	{"slaves",		1, NULL,	's'},
	{"semaphores",		1, NULL,	'l'},
	{"pidfile",		1, NULL,	'i'},
	{"proxy-il-sock",	1, NULL,	'X'},
	{"proxy-il-fprefix",	1, NULL,	'Y'},
#ifdef LB_PERF
	{"perf-sink",		1, NULL,	'K'},
#endif
	{"transactions",	1,	NULL,	'b'},
	{NULL,0,NULL,0}
};

static const char *get_opt_string = "p:c:dm:s:l:i:X:Y:zb:"
#ifdef LB_PERF
	"K:"
#endif
;

static void usage(char *me) 
{
	fprintf(stderr,"usage: %s [option]\n"
		"\t-p, --sock\t\t path-name to the local socket\n"
		"\t-c, --con-queue\t\t size of the connection queue (accept)\n"
		"\t-m, --mysql\t\t database connect string\n"
		"\t-d, --debug\t\t don't run as daemon, additional diagnostics\n"
		"\t-s, --slaves\t\t number of slave servers to fork\n"
		"\t-l, --semaphores\t number of semaphores (job locks) to use\n"
		"\t-i, --pidfile\t\t file to store master pid\n"
		"\t-X, --proxy-il-sock\t socket to send events to\n"
		"\t-Y, --proxy-il-fprefix\t file prefix for events\n"
		"\t-z, --silent\t\t don't print diagnostic, even if -d is on\n"
#ifdef LB_PERF
		"\t-K, --perf-sink\t where to sink events\n"
#endif
		"\t-b, --transactions\t transactions force switch\n"
	,me);
}

static void wait_for_open(edg_wll_Context,const char *);
static int decrement_timeout(struct timeval *, struct timeval, struct timeval);



/*
 *	SERVER BONES structures and handlers
 */
int clnt_data_init(void **);

	/*
	 *	Serve & Store handlers
	 */
int clnt_reject(int);
int handle_conn(int, struct timeval *, void *);
int accept_serve(int, struct timeval *, void *);
int accept_store(int, struct timeval *, void *);
int clnt_disconnect(int, struct timeval *, void *);

#define SRV_SERVE		0
#define SRV_STORE		1
static struct glite_srvbones_service service_table[] = {
	{ "serve",	-1, handle_conn, accept_serve, clnt_reject, clnt_disconnect },
	{ "store",	-1, handle_conn, accept_store, clnt_reject, clnt_disconnect },
};

struct clnt_data_t {
	edg_wll_Context			ctx;
	glite_lbu_DBContext		dbctx;
	int				dbcaps;
};



int main(int argc, char *argv[])
{
	int					i;
	struct sockaddr_un	a;
	int					opt;
	char				pidfile[PATH_MAX] = GLITE_LBPROXY_PIDFILE,
						socket_path_prefix[PATH_MAX] = GLITE_LBPROXY_SOCK_PREFIX,
					   *name;
	FILE			   *fpid;
	key_t				semkey;
	edg_wll_Context		ctx;
	struct timeval		to;
	int	silent = 0;


	name = strrchr(argv[0],'/');
	if (name) name++; else name = argv[0];

	if (geteuid()) snprintf(pidfile,sizeof pidfile,"%s/glite_lb_proxy.pid", getenv("HOME"));

	while ((opt = getopt_long(argc, argv, get_opt_string, opts, NULL)) != EOF) switch (opt) {
		case 'p': strcpy(socket_path_prefix, optarg); break;
		case 'b': transactions = atoi(optarg); break;
		case 'c': con_queue = atoi(optarg); break;
		case 'd': debug = 1; break;
		case 'z': silent = 1; break;
		case 'm': dbstring = optarg; break;
		case 's': slaves = atoi(optarg); break;
		case 'l': semaphores = atoi(optarg); break;
		case 'X': lbproxy_ilog_socket_path = strdup(optarg); break;
		case 'Y': lbproxy_ilog_file_prefix = strdup(optarg); break;
		case 'i': strcpy(pidfile, optarg); break;
#ifdef LB_PERF
		case 'K': sink_mode = atoi(optarg); break;
#endif
		case '?': usage(name); return 1;
	}

	if ( optind < argc ) { usage(name); return 1; }

	setlinebuf(stdout);
	setlinebuf(stderr);

	fpid = fopen(pidfile,"r");
	if ( fpid ) {
		int	opid = -1;

		if ( fscanf(fpid,"%d",&opid) == 1 ) {
			if ( !kill(opid,0) ) {
				fprintf(stderr,"%s: another instance running, pid = %d\n",name,opid);
				return 1;
			}
			else if (errno != ESRCH) { perror("kill()"); return 1; }
		}
		fclose(fpid);
	} else if (errno != ENOENT) { perror(pidfile); return 1; }

	fpid = fopen(pidfile, "w");
	if ( !fpid ) { perror(pidfile); return 1; }
	if (fprintf(fpid, "%d", getpid()) <= 0) { perror(pidfile); return 1; }
	if (fclose(fpid) != 0) { perror(pidfile); return 1; }

	semkey = ftok(pidfile,0);

	if ( semaphores == -1 ) semaphores = slaves;
	semset = semget(semkey, 0, 0);
	if ( semset >= 0 ) semctl(semset, 0, IPC_RMID);
	semset = semget(semkey, semaphores, IPC_CREAT | 0600);
	if ( semset < 0 ) { perror("semget()"); return 1; }
	dprintf(("Using %d semaphores, set id %d\n", semaphores, semset));
	for ( i = 0; i < semaphores; i++ ) {
		struct sembuf	s;

		s.sem_num = i; s.sem_op = 1; s.sem_flg = 0;
		if (semop(semset,&s,1) == -1) { perror("semop()"); return 1; }
	}

	gethostname(host, sizeof host);
	host[sizeof host - 1] = 0;
	asprintf(&port, "%d", GLITE_JOBID_DEFAULT_PORT);
	dprintf(("server address: %s:%s\n", host, port));

	service_table[SRV_SERVE].conn = socket(PF_UNIX, SOCK_STREAM, 0);
	if ( service_table[SRV_SERVE].conn < 0 ) { perror("socket()"); return 1; }
	memset(&a, 0, sizeof(a));
	a.sun_family = AF_UNIX;
	sprintf(sock_serve, "%s%s", socket_path_prefix, "serve.sock");
	strcpy(a.sun_path, sock_serve);

	if( connect(service_table[SRV_SERVE].conn, (struct sockaddr *)&a, sizeof(a.sun_path)) < 0) {
		if( errno == ECONNREFUSED ) {
			dprintf(("removing stale input socket %s\n", sock_serve));
			unlink(sock_serve);
		}
	} else { perror("another instance of lb-proxy is running"); return 1; }

	if ( bind(service_table[SRV_SERVE].conn, (struct sockaddr *) &a, sizeof(a)) < 0 ) {
		char	buf[100];

		snprintf(buf, sizeof(buf), "bind(%s)", sock_serve);
		perror(buf);
		return 1;
	}

	if ( listen(service_table[SRV_SERVE].conn, con_queue) ) { perror("listen()"); return 1; }

	service_table[SRV_STORE].conn = socket(PF_UNIX, SOCK_STREAM, 0);
	if ( service_table[SRV_STORE].conn < 0 ) { perror("socket()"); return 1; }
	memset(&a, 0, sizeof(a));
	a.sun_family = AF_UNIX;
	sprintf(sock_store, "%s%s", socket_path_prefix, "store.sock");
	strcpy(a.sun_path, sock_store);

	if( connect(service_table[SRV_STORE].conn, (struct sockaddr *)&a, sizeof(a.sun_path)) < 0) {
		if( errno == ECONNREFUSED ) {
			dprintf(("removing stale input socket %s\n", sock_store));
			unlink(sock_store);
		}
	} else { perror("another instance of lb-proxy is running"); return 1; }

	if ( bind(service_table[SRV_STORE].conn, (struct sockaddr *) &a, sizeof(a))) {
		char	buf[100];

		snprintf(buf, sizeof(buf), "bind(%s)", sock_store);
		perror(buf);
		return 1;
	}
	if ( listen(service_table[SRV_STORE].conn, con_queue) ) { perror("listen()"); return 1; }

	dprintf(("Listening at %s, %s ...\n", sock_store, sock_serve));

	if (!dbstring) dbstring = getenv("LBPROXYDB");
	if (!dbstring) dbstring = DEFAULTCS;


	/* Just check the database and let it be. The slaves do the job. */
	edg_wll_InitContext(&ctx);
	/* XXX: obsolete
	 * edg_wll_InitContext(&ctx) used to cause segfault
	if ( !(ctx = (edg_wll_Context) malloc(sizeof(*ctx))) ) {
		perror("InitContext()");
		return -1;
	}
	memset(ctx, 0, sizeof(*ctx));
	*/
	wait_for_open(ctx, dbstring);
	if ((ctx->dbcaps = glite_lbu_DBQueryCaps(ctx->dbctx)) == -1)
	{
		char	*et,*ed;
		glite_lbu_DBError(ctx->dbctx,&et,&ed);

		fprintf(stderr,"%s: open database: %s (%s)\n",argv[0],et,ed);
		free(et); free(ed);
		return 1;
	}
	edg_wll_Close(ctx);
	ctx->dbctx = NULL;
	fprintf(stderr, "[%d]: DB '%s'\n", getpid(), dbstring);

	if ((ctx->dbcaps & GLITE_LBU_DB_CAP_INDEX) == 0) {
		fprintf(stderr,"%s: missing index support in DB layer\n",argv[0]);
		return 1;
	}
	if ((ctx->dbcaps & GLITE_LBU_DB_CAP_TRANSACTIONS) == 0)
		fprintf(stderr, "[%d]: transactions aren't supported!\n", getpid());
	if (transactions >= 0) {
		fprintf(stderr, "[%d]: transactions forced from %d to %d\n", getpid(), ctx->dbcaps & GLITE_LBU_DB_CAP_TRANSACTIONS ? 1 : 0, transactions);
		ctx->dbcaps &= ~GLITE_LBU_DB_CAP_TRANSACTIONS;
		ctx->dbcaps |= transactions ? GLITE_LBU_DB_CAP_TRANSACTIONS : 0;
	}
	use_dbcaps = ctx->dbcaps;
	edg_wll_FreeContext(ctx);

	if ( !debug ) {
		if ( daemon(1,0) == -1 ) { perror("deamon()"); exit(1); }

		fpid = fopen(pidfile,"w");
		if ( !fpid ) { perror(pidfile); return 1; }
		fprintf(fpid, "%d", getpid());
		fclose(fpid);
		openlog(name, LOG_PID, LOG_DAEMON);
	} else { setpgid(0, getpid()); }

	if (silent) debug = 0;

	glite_srvbones_set_param(GLITE_SBPARAM_SLAVES_COUNT, slaves);
	glite_srvbones_set_param(GLITE_SBPARAM_SLAVE_OVERLOAD, SLAVE_OVERLOAD);
	glite_srvbones_set_param(GLITE_SBPARAM_SLAVE_CONNS_MAX, SLAVE_CONNS_MAX);
	to = (struct timeval){REQUEST_TIMEOUT, 0};
	glite_srvbones_set_param(GLITE_SBPARAM_REQUEST_TIMEOUT, &to);
	to = (struct timeval){IDLE_TIMEOUT, 0};
	glite_srvbones_set_param(GLITE_SBPARAM_IDLE_TIMEOUT, &to);

	glite_srvbones_run(clnt_data_init, service_table, sizofa(service_table), debug);

	semctl(semset, 0, IPC_RMID, 0);
	unlink(pidfile);
	for ( i = 0; i < sizofa(service_table); i++ )
		if ( service_table[i].conn >= 0 ) close(service_table[i].conn);
	unlink(sock_serve);
	unlink(sock_store);
	if (port) free(port);

	return 0;
}


int clnt_data_init(void **data)
{
	edg_wll_Context			ctx;
	struct clnt_data_t	   *cdata;


	if ( !(cdata = calloc(1, sizeof(*cdata))) )
		return -1;

	if ( !(ctx = (edg_wll_Context) malloc(sizeof(*ctx))) ) { free(cdata); return -1; }
	memset(ctx, 0, sizeof(*ctx));

	dprintf(("[%d] opening database ...\n", getpid()));
	wait_for_open(ctx, dbstring);
	cdata->dbctx = ctx->dbctx;
	cdata->dbcaps = use_dbcaps;
	edg_wll_FreeContext(ctx);

#ifdef LB_PERF
	glite_wll_perftest_init(NULL, NULL, NULL, NULL, 0);
#endif

	*data = cdata;
	return 0;
}

	
int handle_conn(int conn, struct timeval *timeout, void *data)
{
	struct clnt_data_t *cdata = (struct clnt_data_t *)data;
	edg_wll_Context		ctx;
	struct timeval		conn_start, now;

        if ( edg_wll_InitContext(&ctx) ) {
		dprintf(("Couldn't create context"));
		return -1;
	}
	cdata->ctx = ctx;

	/* Shared structures (pointers)
	 */
	ctx->dbctx = cdata->dbctx;
	ctx->dbcaps = cdata->dbcaps;
	
	/*	set globals
	 */
	ctx->allowAnonymous = 1;
	ctx->isProxy = 1;
	ctx->noAuth = 1;
	ctx->noIndex = 1;
	ctx->semset = semset;
	ctx->semaphores = semaphores;

	ctx->srvName = strdup(host);
	ctx->srvPort = atoi(port);
	
	ctx->connProxy = (edg_wll_ConnProxy *) calloc(1, sizeof(edg_wll_ConnProxy));
	if ( !ctx->connProxy ) {
		perror("calloc");
		edg_wll_FreeContext(ctx);

		return -1;
	}

	gettimeofday(&conn_start, 0);
	if ( edg_wll_plain_accept(conn, &ctx->connProxy->conn) ) {
		perror("accept");
		edg_wll_FreeContext(ctx);

		return -1;
	} 

	gettimeofday(&now, 0);
	if ( decrement_timeout(timeout, conn_start, now) ) {
		if (debug) fprintf(stderr, "edg_wll_plain_accept() timeout");
		else syslog(LOG_ERR, "edg_wll_plain_accept(): timeout");

		return -1;
	}


	return 0;
}


int accept_store(int conn, struct timeval *timeout, void *cdata)
{
	edg_wll_Context		ctx = ((struct clnt_data_t *) cdata)->ctx;
	struct timeval		before, after;
	char			   *errt, *errd;
	int					err;

	memcpy(&ctx->p_tmp_timeout, timeout, sizeof(ctx->p_tmp_timeout));
	gettimeofday(&before, NULL);
	errt = errd = NULL;
	if ( edg_wll_StoreProtoProxy(ctx) ) {
		switch ( (err = edg_wll_Error(ctx, &errt, &errd)) ) {
		case ETIMEDOUT:
		case EPIPE:
			dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
			if (!debug) syslog(LOG_ERR,"%s (%s)", errt, errd);
			/*	fallthrough
			 */
		case ENOTCONN:
			free(errt); free(errd);
			return err;
			break;

		case ENOENT:
		case EINVAL:
		case EPERM:
		case EEXIST:
		case EDG_WLL_ERROR_NOINDEX:
		case E2BIG:
			dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
			if ( !debug ) syslog(LOG_ERR, "%s (%s)", errt, errd);
			break;
			
		default:
			dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
			if ( !debug ) syslog(LOG_CRIT, "%s (%s)", errt, errd);
			return -1;
		} 
		free(errt); free(errd);
	} else if ( edg_wll_Error(ctx, &errt, &errd) ) { 
		dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
		if ( !debug ) syslog(LOG_ERR, "%s (%s)", errt, errd);
		free(errt); free(errd);
		edg_wll_ResetError(ctx);
	}
	gettimeofday(&after, NULL);
	if ( decrement_timeout(timeout, before, after) ) {
		if (debug) fprintf(stderr, "Serving store connection timed out");
		else syslog(LOG_ERR, "Serving store connection timed out");
		return ETIMEDOUT;
	}

	return 0;
}

int accept_serve(int conn, struct timeval *timeout, void *cdata)
{
	edg_wll_Context		ctx = ((struct clnt_data_t *) cdata)->ctx;
	struct timeval		before, after;


	/*
	 *	serve the request
	 */
	memcpy(&ctx->p_tmp_timeout, timeout, sizeof(ctx->p_tmp_timeout));
	gettimeofday(&before, NULL);
	if ( edg_wll_ServerHTTP(ctx) ) { 
		char    *errt, *errd;
		int		err;

		
		errt = errd = NULL;
		switch ( (err = edg_wll_Error(ctx, &errt, &errd)) ) {
		case ETIMEDOUT:
		case EPIPE:
		case EIO:
		case EDG_WLL_IL_PROTO:
			dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
			if (!debug) syslog(LOG_ERR,"%s (%s)", errt, errd);
			/*	fallthrough
			 */
		case ENOTCONN:
			free(errt); free(errd);
			return err;
			break;

		case ENOENT:
		case EPERM:
		case EEXIST:
		case EDG_WLL_ERROR_NOINDEX:
		case E2BIG:
			dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
			break;
		case EINVAL:
		case EDG_WLL_ERROR_PARSE_BROKEN_ULM:
		case EDG_WLL_ERROR_PARSE_EVENT_UNDEF:
		case EDG_WLL_ERROR_PARSE_MSG_INCOMPLETE:
		case EDG_WLL_ERROR_PARSE_KEY_DUPLICITY:
		case EDG_WLL_ERROR_PARSE_KEY_MISUSE:
		case EDG_WLL_ERROR_PARSE_OK_WITH_EXTRA_FIELDS:
		case EDG_WLL_ERROR_JOBID_FORMAT:
		case EDG_WLL_ERROR_MD5_CLASH:
			dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
			if ( !debug ) syslog(LOG_ERR,"%s (%s)", errt, errd);
			/*
			 *	no action for non-fatal errors
			 */
			break;
			
		case EDG_WLL_ERROR_DB_CALL:
		case EDG_WLL_ERROR_SERVER_RESPONSE:
		default:
			dprintf(("[%d] %s (%s)\n", getpid(), errt, errd));
			if (!debug) syslog(LOG_CRIT,"%s (%s)",errt,errd);
			/*
			 *	unknown error - do rather return (<0) (slave will be killed)
			 */
			return -1;
		} 
		free(errt); free(errd);
	}
	gettimeofday(&after, NULL);
	if ( decrement_timeout(timeout, before, after) ) {
		if (debug) fprintf(stderr, "Serving store connection timed out");
		else syslog(LOG_ERR, "Serving store connection timed out");
		return ETIMEDOUT;
	}

	return 0;
}


int clnt_disconnect(int conn, struct timeval *timeout, void *cdata)
{
	edg_wll_Context		ctx = ((struct clnt_data_t *) cdata)->ctx;

	/* XXX: handle the timeout
	 */
    if ( ctx->connProxy && ctx->connProxy->conn.sock >= 0 )
		edg_wll_plain_close(&ctx->connProxy->conn);

	edg_wll_FreeContext(ctx);
	ctx = NULL;

	return 0;
}

int clnt_reject(int conn)
{
	return 0;
}

static void wait_for_open(edg_wll_Context ctx, const char *dbstring)
{
	char	*dbfail_string1, *dbfail_string2;

	dbfail_string1 = dbfail_string2 = NULL;

	while (edg_wll_Open(ctx, (char *) dbstring)) {
		char	*errt,*errd;

		if (dbfail_string1) free(dbfail_string1);
		glite_lbu_DBError(ctx->dbctx,&errt,&errd);
		asprintf(&dbfail_string1,"%s (%s)\n",errt,errd);
		if (dbfail_string1 != NULL) {
			if (dbfail_string2 == NULL || strcmp(dbfail_string1,dbfail_string2)) {
				if (dbfail_string2) free(dbfail_string2);
				dbfail_string2 = dbfail_string1;
				dbfail_string1 = NULL;
				dprintf(("[%d]: %s\nStill trying ...\n",getpid(),dbfail_string2));
				if (!debug) syslog(LOG_ERR,dbfail_string2);
			}
		}
		sleep(5);
	}

	if (dbfail_string1) free(dbfail_string1);
	if (dbfail_string2 != NULL) {
		free(dbfail_string2);
		dprintf(("[%d]: DB connection established\n",getpid()));
		if (!debug) syslog(LOG_INFO,"DB connection established\n");
	}
}

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

