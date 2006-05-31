#!/bin/bash

numjobs=$1

# XXX - there must be better way to find stage
STAGEDIR=/home/michal/shared/egee/jra1-head/stage
. $STAGEDIR/sbin/perftest_common.sh

DEBUG=${DEBUG:-0}
PERFTEST_CONSUMER=./glite_lb_proxy_perf 
# CONSUMER_ARGS=
# PERFTEST_COMPONENT=
# COMPONENT_ARGS=
#LOGJOBS_ARGS="" 

check_test_files || exit 1

echo -e "\tsmall_job \t big_job \t small_dag \t big_dag"
i=1
while [[ $i -lt 5 ]]
do
	CONSUMER_ARGS="-d --perf-sink $i" 
	echo Running test $i
	run_test proxy $numjobs
	j=0
	while [[ $j -lt 4 ]]
	do
		echo -e -n "\t ${PERFTEST_THROUGHPUT[$j]}"	
		j=$((j+1))
	done
	echo ""
	# purge jobs from database
	$LOGJOBS -n $numjobs > /tmp/perftest.jobids
	i=$((i+1))
done

