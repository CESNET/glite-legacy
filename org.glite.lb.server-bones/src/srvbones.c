#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <assert.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>

#include "srvbones.h"

#define SLAVES_CT			5		/* default number of slaves */
#define SLAVE_OVERLOAD		10		/* queue items per slave */
#define SLAVE_CONNS_MAX		500		/* commit suicide after that many connections */
#define SLAVE_CHECK_SIGNALS	2		/* how often to check signals while waiting for recv_mesg */
#define CLNT_TIMEOUT		10		/* keep idle connection that many seconds */
#define TOTAL_CLNT_TIMEOUT	60		/* one client may ask one slave multiple times */
									/* but only limited time to avoid DoS attacks */

#ifndef dprintf
#define dprintf(x)			{ if (debug) printf x; }
#endif


static int					running = 0;
static int					debug = 0;
static volatile int			die = 0,
							child_died = 0;
static unsigned long		clnt_dispatched = 0,
							clnt_accepted = 0;

static struct glite_srvbones_service  *services;
static int					services_ct;

static int					set_slaves_ct = SLAVES_CT;
static int					set_slave_overload = SLAVE_OVERLOAD;
static int					set_slave_conns_max = SLAVE_CONNS_MAX;
static struct timeval		set_clnt_to = {CLNT_TIMEOUT, 0};
static struct timeval		set_total_clnt_to = {TOTAL_CLNT_TIMEOUT, 0};


static int dispatchit(int, int, int);
static int do_sendmsg(int, int, unsigned long, int);
static int do_recvmsg(int, int *, unsigned long *, int *);
static int check_timeout(struct timeval *, struct timeval, struct timeval);
static void catchsig(int);
static void catch_chld(int sig);
static int slave(int (*)(void **), int);

static void glite_srvbones_set_slaves_ct(int);
static void glite_srvbones_set_slave_overload(int);
static void glite_srvbones_set_slave_conns_max(int);
static void glite_srvbones_set_clnt_to(struct timeval *);
static void glite_srvbones_set_total_clnt_to(struct timeval *);


int glite_srvbones_set_param(glite_srvbones_param_t param, ...)
{
	va_list ap;

	if ( running ) {
		dprintf(("Attempting to set srv-bones parameter on running server"));
		return -1;
	}

	va_start(ap, param);
	switch ( param ) {
	case GLITE_SBPARAM_SLAVES_CT:
		glite_srvbones_set_slaves_ct(va_arg(ap,int)); break;
	case GLITE_SBPARAM_SLAVE_OVERLOAD:
		glite_srvbones_set_slave_overload(va_arg(ap,int)); break;
	case GLITE_SBPARAM_SLAVE_CONNS_MAX:
		glite_srvbones_set_slave_conns_max(va_arg(ap,int)); break;
	case GLITE_SBPARAM_CLNT_TIMEOUT:
		glite_srvbones_set_clnt_to(va_arg(ap,struct timeval *)); break;
	case GLITE_SBPARAM_TOTAL_CLNT_TIMEOUT:
		glite_srvbones_set_total_clnt_to(va_arg(ap,struct timeval *)); break;
	}
	va_end(ap);

	return 0;
}

int glite_srvbones_run(
	slave_data_init_hnd				slave_data_init,
	struct glite_srvbones_service  *service_table,
	size_t							table_sz,
	int								dbg)
{
	struct sigaction	sa;
	sigset_t			sset;
	int					sock_slave[2], i;


	assert(service_table);
	assert(table_sz > 0);

	services = service_table;
	services_ct = table_sz;
	debug = dbg;

	setlinebuf(stdout);
	setlinebuf(stderr);
	dprintf(("Master pid %d\n", getpid()));

	if ( socketpair(AF_UNIX, SOCK_STREAM, 0, sock_slave) )
	{
		perror("socketpair()");
		return 1;
	}

	memset(&sa, 0, sizeof(sa)); assert(sa.sa_handler == NULL);
	sa.sa_handler = catchsig;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	sa.sa_handler = catch_chld;
	sigaction(SIGCHLD, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGUSR1, &sa, NULL);

	sigemptyset(&sset);
	sigaddset(&sset, SIGCHLD);
	sigaddset(&sset, SIGTERM);
	sigaddset(&sset, SIGINT);
	sigprocmask(SIG_BLOCK, &sset, NULL);

	for ( i = 0; i < set_slaves_ct; i++ )
		slave(slave_data_init, sock_slave[1]);

	while ( !die )
	{
		fd_set			fds;
		int				ret, mx;
		

		FD_ZERO(&fds);
		FD_SET(sock_slave[0], &fds);
		for ( i = 0, mx = sock_slave[0]; i < services_ct; i++ )
		{
			FD_SET(services[i].conn, &fds);
			if ( mx < services[i].conn ) mx = services[i].conn;
		}

		sigprocmask(SIG_UNBLOCK, &sset, NULL);
		ret = select(mx+1, &fds, NULL, NULL, NULL);
		sigprocmask(SIG_BLOCK, &sset, NULL);

		if ( ret == -1 && errno != EINTR )
		{
			if ( debug ) perror("select()");
			else syslog(LOG_CRIT,"select(): %m");

			return 1;
		}

		if ( child_died )
		{
			int		pid;

			while ( (pid = waitpid(-1, NULL, WNOHANG)) > 0 )
			{
				if ( !die )
				{
					int newpid = slave(slave_data_init, sock_slave[1]);
					dprintf(("[master] Servus mortuus [%d] miraculo resurrexit [%d]\n", pid, newpid));
				}
			}
			child_died = 0;
			continue;
		}

		if ( die ) continue;

		
		if (FD_ISSET(sock_slave[0],&fds)) {
			/* slave accepted a request
			 */
			unsigned long	a;

			if (    (recv(sock_slave[0], &a, sizeof(a), MSG_WAITALL) == sizeof(a))
				 && (a <= clnt_dispatched)
				 && (a > clnt_accepted || clnt_accepted > clnt_dispatched) )
				clnt_accepted = a;
		}

		for ( i = 0; i < services_ct; i++ )
			if (   FD_ISSET(services[i].conn, &fds)
				&& dispatchit(sock_slave[0], services[i].conn ,i) )
				/* Be carefull!!!
				 * This must break this for cykle but start the
				 * while (!die) master cykle from the top also
				 */
				break;
	}

	dprintf(("[master] Terminating on signal %d\n", die));
	if (!debug) syslog(LOG_INFO, "Terminating on signal %d\n", die);
	kill(0, die);

	return 0;
}

static int dispatchit(int sock_slave, int sock, int sidx)
{
	struct sockaddr_in	a;
	unsigned char	   *pom;
	int					conn,
						alen, ret;


	alen = sizeof(a);
	if ( (conn = accept(sock, (struct sockaddr *)&a, &alen)) < 0 )
	{ 
		if (debug)
		{
			perror("accept()");
			return 1; 
		}
		else
		{
			syslog(LOG_ERR, "accept(): %m");
			sleep(5);
			return -1;
		}
	}

	getpeername(conn, (struct sockaddr *)&a, &alen);
	pom = (char *) &a.sin_addr.s_addr;
	dprintf(("[master] %s connection from %d.%d.%d.%d:%d\n",
				services[sidx].id? services[sidx].id: "",
				(int)pom[0], (int)pom[1], (int)pom[2], (int)pom[3],
				ntohs(a.sin_port)));

	ret = 0;
	if (    (   clnt_dispatched < clnt_accepted	/* wraparound */
		     || clnt_dispatched - clnt_accepted < set_slaves_ct * set_slave_overload)
		&& !(ret = do_sendmsg(sock_slave, conn, clnt_dispatched++, sidx)) )
	{
		/*	all done
		 */ 
		dprintf(("[master] Dispatched %lu, last known served %lu\n",
				clnt_dispatched-1, clnt_accepted));
	}
	else
	{
		services[sidx].on_reject_hnd(conn);
		dprintf(("[master] Reject due to overload\n"));
	}

	close(conn);
	if (ret)
	{
		perror("sendmsg()");
		if ( !debug ) syslog(LOG_ERR, "sendmsg(): %m");
	}


	return 0;
}


static int slave(slave_data_init_hnd data_init_hnd, int sock)
{
	sigset_t			sset;
	struct sigaction	sa;
	struct timeval		client_done,
						client_start;
	void			   *clnt_data = NULL;
	int					conn = -1,
						srv = -1,
						conn_cnt = 0,
						sockflags,
						h_errno,
						pid, i;



	if ( (pid = fork()) ) return pid;

	srandom(getpid()+time(NULL));

	for ( i = 0; i < services_ct; i++ )
		close(services[i].conn);

	sigemptyset(&sset);
	sigaddset(&sset, SIGTERM);
	sigaddset(&sset, SIGINT);
	sigaddset(&sset, SIGUSR1);

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catchsig;
	sigaction(SIGUSR1, &sa, NULL);

	if (   (sockflags = fcntl(sock, F_GETFL, 0)) < 0
		|| fcntl(sock, F_SETFL, sockflags | O_NONBLOCK) < 0 )
	{
		dprintf(("[%d] fcntl(master_sock): %s\n", getpid(), strerror(errno)));
		if ( !debug ) syslog(LOG_CRIT, "fcntl(master_sock): %m");
		exit(1);
	}

	if ( data_init_hnd && data_init_hnd(&clnt_data) )
		/*
		 *	XXX: what if the error remains and master will start new slave
		 *	again and again?
		 */
		exit(1);

	while ( !die && (conn_cnt < set_slave_conns_max || conn >= 0) )
	{
		fd_set				fds;
		int					max = sock,
							connflags,
							newconn = -1,
							newsrv = -1,
							kick_client = 0;
		unsigned long		seq;
		struct timeval		check_to = { SLAVE_CHECK_SIGNALS, 0},
							total_to = set_total_clnt_to,
							client_to = set_clnt_to,
							now;


		FD_ZERO(&fds);
		FD_SET(sock, &fds);
		if ( conn >= 0 ) FD_SET(conn, &fds);
		if ( conn > sock ) max = conn;
	
		sigprocmask(SIG_UNBLOCK, &sset, NULL);
		switch ( select(max+1, &fds, NULL, NULL, &check_to) )
		{
		case -1:
			if ( errno != EINTR )
			{
				dprintf(("[%d] select(): %s\n", getpid(), strerror(errno)));
				if ( !debug ) syslog(LOG_CRIT, "select(): %m");
				exit(1);
			}
			continue;
			
		case 0:
			if ( conn < 0 ) continue;
			
		default:
			break;
		}
		sigprocmask(SIG_BLOCK, &sset, NULL);

		gettimeofday(&now,NULL);
		if (   conn >= 0
			&& (   check_timeout(&client_to, client_done, now)
				|| check_timeout(&total_to, client_start, now)) )
			kick_client = 1;

		if ( conn >= 0 && !kick_client && FD_ISSET(conn, &fds) )
		{
			/*
			 *	serve the request
			 */
			int		rv;

			dprintf(("[%d] incoming request\n", getpid()));
			if ( !services[srv].on_accept_hnd )
			{
				dprintf(("[%d] request handler for '%s' service not set\n", getpid(), services[srv].id));
				kick_client = 1;
				continue;
			}

			if ( (rv = services[srv].on_accept_hnd(conn, clnt_data)) > 0 )
			{
				/*	expected FATAL error -> close connection and contiue
				 */
				close(conn);
				conn = -1;
				continue;
			}
			else if ( rv < 0 )
				/*	unknown error -> clasified as FATAL -> kill slave
				 */
				exit(1);

			dprintf(("[%d] request done\n", getpid()));
			gettimeofday(&client_done, NULL);
			continue;
		}

		if ( FD_ISSET(sock, &fds) && conn_cnt < set_slave_conns_max )
		{
			if ( conn >= 0 ) usleep(100000 + 1000 * (random() % 200));
			if ( do_recvmsg(sock, &newconn, &seq, &newsrv) ) switch ( errno )
			{
			case EINTR: /* XXX: signals are blocked */
		 	case EAGAIN:
				continue;
			default: dprintf(("[%d] recvmsg(): %s\n", getpid(), strerror(errno)));
				if (!debug) syslog(LOG_CRIT,"recvmsg(): %m\n");
				exit(1);
			}
			kick_client = 1;
		}

		if ( kick_client && conn >= 0 )
		{
			if ( services[srv].on_disconnect_hnd )
				services[srv].on_disconnect_hnd(conn, clnt_data);
			close(conn);
			conn = -1;
			srv = -1;
			dprintf(("[%d] Idle connection closed\n", getpid()));
		}

		if ( newconn >= 0 )
		{
			conn = newconn;
			srv = newsrv;
			gettimeofday(&client_start, NULL);
			client_done.tv_sec = client_start.tv_sec;
			client_done.tv_usec = client_start.tv_usec;

			switch ( send(sock, &seq, sizeof(seq), 0) )
			{
			case -1:
				if (debug) perror("send()");
				else syslog(LOG_CRIT, "send(): %m\n");
				exit(1);
				
			case sizeof(seq):
				break;
				
			default: dprintf(("[%d] send(): incomplete message\n", getpid()));
				exit(1);
			}
	
			conn_cnt++;
			dprintf(("[%d] serving %s connection %lu\n", getpid(),
					services[srv].id? services[srv].id: "", seq));
	
			connflags = fcntl(conn, F_GETFL, 0);
			if ( fcntl(conn, F_SETFL, connflags | O_NONBLOCK) < 0 )
			{
				dprintf(("[%d] can't set O_NONBLOCK mode (%s), closing.\n", getpid(), strerror(errno)));
				if ( !debug ) syslog(LOG_ERR, "can't set O_NONBLOCK mode (%s), closing.\n", strerror(errno));
				close(conn);
				conn = srv = -1;
				continue;
			}

			if (   services[srv].on_new_conn_hnd
				&& services[srv].on_new_conn_hnd(conn, client_start, clnt_data) )
			{
				dprintf(("[%d] Connection not estabilished.\n", getpid()));
				if ( !debug ) syslog(LOG_ERR, "Connection not estabilished.\n");
				close(conn);
				conn = srv = -1;
				continue;
			}
		}
	}

	if ( die )
	{
		dprintf(("[%d] Terminating on signal %d\n", getpid(), die));
		if ( !debug ) syslog(LOG_INFO, "Terminating on signal %d", die);
	}
	dprintf(("[%d] Terminating after %d connections\n", getpid(), conn_cnt));
	if ( !debug ) syslog(LOG_INFO, "Terminating after %d connections", conn_cnt);


	exit(0);
}

static void catchsig(int sig)
{
	die = sig;
}

static void catch_chld(int sig)
{
	child_died = 1;
}

static int check_timeout(struct timeval *timeout, struct timeval before, struct timeval after)
{
	return (timeout->tv_usec <= after.tv_usec - before.tv_usec) ? 
			(timeout->tv_sec <= after.tv_sec - before.tv_sec) :
			(timeout->tv_sec < after.tv_sec - before.tv_sec);
}

#define MSG_BUFSIZ	30

/*
 * send socket sock through socket to_sock
 */
static int do_sendmsg(int to_sock, int sock, unsigned long clnt_dispatched, int srv)
{
	struct msghdr		msg = {0};
	struct cmsghdr	   *cmsg;
	struct iovec		sendiov;
	int					myfds,							/* file descriptors to pass. */
					   *fdptr;
	char				buf[CMSG_SPACE(sizeof myfds)];	/* ancillary data buffer */
	char				sendbuf[MSG_BUFSIZ];			/* to store unsigned int + \0 */


	snprintf(sendbuf, sizeof(sendbuf), "%u %lu", srv, clnt_dispatched);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &sendiov;
	msg.msg_iovlen = 1;
	sendiov.iov_base = sendbuf;
	sendiov.iov_len = sizeof(sendbuf);

	msg.msg_control = buf;
	msg.msg_controllen = sizeof buf;

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	fdptr = (int *)CMSG_DATA(cmsg);
	*fdptr = sock;

	msg.msg_controllen = cmsg->cmsg_len;
	/* send fd to server-slave to do rest of communication */
	if (sendmsg(to_sock, &msg, 0) < 0)  
		return 1;
        
	return 0;
}


/* receive socket sock through socket from_sock */
static int do_recvmsg(int from_sock, int *sock, unsigned long *clnt_accepted,int *srv)
{
	struct msghdr		msg = {0};
	struct cmsghdr	   *cmsg;
	struct iovec		recviov;
	int					myfds;							/* file descriptors to pass. */
	char				buf[CMSG_SPACE(sizeof(myfds))];	/* ancillary data buffer */
	char				recvbuf[MSG_BUFSIZ];


	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &recviov;
	msg.msg_iovlen = 1;
	recviov.iov_base = recvbuf;
	recviov.iov_len = sizeof(recvbuf);

	msg.msg_control = buf;
	msg.msg_controllen = sizeof buf;

	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	msg.msg_controllen = cmsg->cmsg_len;

	if (recvmsg(from_sock, &msg, 0) < 0) 
		return 1;
        
	*sock = *((int *)CMSG_DATA(cmsg));
	sscanf(recvbuf, "%u %lu", srv, clnt_accepted);

	return 0;
}

static void glite_srvbones_set_slaves_ct(int n)
{
	set_slaves_ct = (n == -1)? SLAVES_CT: n;
}

static void glite_srvbones_set_slave_overload(int n)
{
	set_slave_overload = (n == -1)? SLAVE_OVERLOAD: n;
}

static void glite_srvbones_set_slave_conns_max(int n)
{
	set_slave_conns_max = (n == -1)? SLAVE_CONNS_MAX: n;
}

static void glite_srvbones_set_clnt_to(struct timeval *t)
{
	set_clnt_to = t? (struct timeval){CLNT_TIMEOUT, 0}: *t;
}

static void glite_srvbones_set_total_clnt_to(struct timeval *t)
{
	set_total_clnt_to = t? (struct timeval){TOTAL_CLNT_TIMEOUT, 0}: *t;
}
