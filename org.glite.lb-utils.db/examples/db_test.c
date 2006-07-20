#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"


#define dprintf(ARGS) { printf("%s: ", name); printf ARGS; }


int main(int argn, char *argv[]) {
	char *name;
	const char *cs;
	glite_lbu_DBContext ctx;
	glite_lbu_Statement stmt;
	int caps;

	if ((name = strrchr(argv[0], '/')) != NULL) name++;
	else name = argv[0];
	if ((cs = getenv("DB")) == NULL) cs = "jpis/@localhost:jpis1";

	// init
	dprintf(("connecting to %s...\n", cs));
	if (glite_lbu_DBConnect(&ctx, cs, 0) != 0) goto fail;
	if ((caps = glite_lbu_DBQueryCaps(ctx)) == -1) goto failctx;
	if ((caps & GLITE_LBU_DB_CAP_PREPARED) == 0) {
		dprintf(("can't do prepared commands, exiting."));
		goto failctx;
	}
	glite_lbu_DBSetCaps(ctx, caps);
	dprintf(("capabilities: %d\n", caps));
	dprintf(("\n"));

	// "trio" queries
{
	int nr, i;
	char **res;

	dprintf(("selecting...\n"));
	if ((glite_lbu_ExecSQL(ctx, "SELECT uniqueid, feedid, state, source, condition FROM feeds", &stmt)) == -1) goto failctx;

	dprintf(("fetching...\n"));
	res = calloc(6, sizeof(char *));
	while ((nr = glite_lbu_FetchRow(stmt, 5, NULL, res)) > 0) {
		dprintf(("Result: n=%d, res=%p\n", nr, res));
		i = 0;
		if (res) {
			dprintf(("  uniqueid  = %s\n", res[0]));
			dprintf(("  feedid    = %s\n", res[1]));
			dprintf(("  state     = %s\n", res[2]));
			dprintf(("  source    = %s\n", res[3]));
			dprintf(("  condition = %s\n", res[4]));
			while(i < nr) {free(res[i]);i++;}
		}
	}
	free(res);
	dprintf(("closing stmt...\n"));
	dprintf(("\n"));
	glite_lbu_FreeStmt(stmt);
}

	// "param" queries
{
	int nr, i;
	char **res;
	long int param_state;

	dprintf(("preparing...\n"));
	if ((glite_lbu_PrepareStmt(ctx, "SELECT feedid, state, source, condition FROM feeds WHERE state = ?", &stmt)) != 0) goto failctx;

	param_state = 1;
	dprintf(("executing state %ld...\n", param_state));
	if (glite_lbu_ExecStmt(stmt, 1, GLITE_LBU_DB_TYPE_INT, param_state) == -1) goto failstmt;
	dprintf(("fetching...\n"));
	res = calloc(5, sizeof(char *));
	while ((nr = glite_lbu_FetchRow(stmt, 4, NULL, res)) > 0) {
		dprintf(("Result: n=%d, res=%p\n", nr, res));
		i = 0;
		if (res) {
			dprintf(("  feedid=%s\n", res[0]));
			dprintf(("  state=%s\n", res[1]));
			dprintf(("  source=%s\n", res[2]));
			dprintf(("  condition=%s\n", res[3]));
			while(i < nr) {free(res[i]);i++;}
		}
	}
	free(res);
	dprintf(("\n"));

	param_state = 3;
	dprintf(("executing state %ld...\n", param_state));
	if (glite_lbu_ExecStmt(stmt, 1, GLITE_LBU_DB_TYPE_INT, param_state) == -1) goto failstmt;
	dprintf(("fetching...\n"));
	res = calloc(5, sizeof(char *));
	while ((nr = glite_lbu_FetchRow(stmt, 4, NULL, res)) > 0) {
		dprintf(("Result: n=%d, res=%p\n", nr, res));
		i = 0;
		if (res) {
			dprintf(("  feedid=%s\n", res[0]));
			dprintf(("  state=%s\n", res[1]));
			dprintf(("  source=%s\n", res[2]));
			dprintf(("  condition=%s\n", res[3]));
			while(i < nr) {free(res[i]);i++;}
		}
	}
	free(res);
	dprintf(("\n"));

	dprintf(("closing stmt...\n"));
	glite_lbu_FreeStmt(stmt);
	dprintf(("\n"));
}

	dprintf(("closing...\n"));
	glite_lbu_DBClose(ctx);
	return 0;

failstmt:
	printf("closing stmt...\n");
	glite_lbu_FreeStmt(stmt);
failctx:
	dprintf(("closing...\n"));
	glite_lbu_DBClose(ctx);
fail:
	dprintf(("failed\n"));
	return 1;
}
