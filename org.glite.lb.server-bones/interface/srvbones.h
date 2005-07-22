#ifndef __ORG_GLITE_LB_SERVER_BONES_BONES_H__
#define __ORG_GLITE_LB_SERVER_BONES_BONES_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _glite_srvbones_param_t {
	GLITE_SBPARAM_SLAVES_CT,			/* default number of slaves */ 
	GLITE_SBPARAM_SLAVE_OVERLOAD,		/* queue items per slave */
	GLITE_SBPARAM_SLAVE_CONNS_MAX,		/* commit suicide after that many */
										/* connections */
	GLITE_SBPARAM_CLNT_TIMEOUT,			/* keep idle connection that many */
										/* seconds */
	GLITE_SBPARAM_TOTAL_CLNT_TIMEOUT,	/* client may ask one slave multiple */
										/* times but only limited time to */
										/* avoid DoS attacks */
} glite_srvbones_param_t;

typedef int (*slave_data_init_hnd)(void **);

struct glite_srvbones_service {
	char	   *id;
	int			conn;
	int		  (*on_new_conn_hnd)(int conn, struct timeval start, void *clnt_data);
	int		  (*on_accept_hnd)(int conn, void *clnt_data);
	int		  (*on_reject_hnd)(int conn);
	int		  (*on_disconnect_hnd)(int conn, void *clnt_data);
};

extern int glite_srvbones_set_param(glite_srvbones_param_t param, ...);
/*
 *	slaves_ct - forked slaves count
 *	slave_data_init_hnd - callback initializing client data on every slave
 */
extern int glite_srvbones_run(
	slave_data_init_hnd				slave_data_init,
	struct glite_srvbones_service  *service_table,
	size_t							table_sz,
	int								dbg);

#ifdef __cplusplus
}
#endif

#endif /* __ORG_GLITE_LB_SERVER_BONES_BONES_H__ */
