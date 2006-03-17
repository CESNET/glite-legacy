#ident "$Header$"

#include <stdio.h>

#include "glite/wmsutils/jobid/cjobid.h"
#include "glite/lb/producer.h"
#include "glite/lb/jobstat.h"

char* write2rgma_statline(edg_wll_JobStat *stat)
{
	fputs("fake write2rgma_statline()\n",stderr);
	return NULL;
}

void write2rgma_status(edg_wll_JobStat *stat)
{
	fputs("fake write2rgma_statline()\n",stderr);
}

void write2rgma_chgstatus(edg_wll_JobStat *stat, char *prev_statline)
{
	fputs("fake write2rgma_chgstatus()\n",stderr);
}
