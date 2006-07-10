#!/bin/bash

numjobs=$1

# XXX - there must be better way to find stage
STAGEDIR=/home/michal/shared/egee/jra1-head/stage
. $STAGEDIR/sbin/perftest_common.sh

LOGEVENT=${LOGEVENT:-$STAGEDIR/bin/glite-lb-logevent}

DEBUG=${DEBUG:-0}
PERFTEST_CONSUMER=$STAGEDIR/bin/glite-lb-proxy

# CONSUMER_ARGS=
# PERFTEST_COMPONENT=
# COMPONENT_ARGS=
#LOGJOBS_ARGS="" 

check_test_files || exit 1
check_file_executable $LOGEVENT || exit 1

SEQCODE="UI=999990:NS=9999999990:WM=999990:BH=9999999990:JSS=999990:LM=999990:LRMS=999990:APP=999990"

purge_proxy ()
{
    for jobid in $@
    do
	$LOGEVENT -x -S /tmp/proxy.perfstore.sock -c $SEQCODE -j $jobid -s UserInterface -e Abort --reason Purge > /dev/null 2>&1
    done
}

echo "----------------------------------"
echo "LB Proxy test"
echo "----------------------------------"
echo "Events are consumed:"
echo "1) before parsing"
echo "2) after parsing, before storing into database"
echo "3) after storing into db, before computing state"
echo "4) after computing state, before sending to IL"
echo "5) by IL"
echo ""
echo -e "\tavg_job \t big_job \t avg_dag \t big_dag"


LOGJOBS_ARGS="-s /tmp/proxy.perf"

i=1
while [[ $i -lt 5 ]]
do
	CONSUMER_ARGS="-d --perf-sink $i -p /tmp/proxy.perf" 
	export PERFTEST_NAME="proxy_test_$i"
	echo -n "${i})"
	run_test proxy $numjobs
	print_result
	# purge jobs from database
	# we have to start proxy again 
	$PERFTEST_CONSUMER -d -p /tmp/proxy.perf -s 1 >/dev/null 2>&1  &
	PID=$!
	purge_proxy `$LOGJOBS -n $numjobs`
	sleep 2
	shutdown $PID
	i=$((i+1))
done

PERFTEST_COMPONENT="$STAGEDIR/bin/glite-lb-proxy"
COMPONENT_ARGS="-d -p /tmp/proxy.perf --proxy-il-sock /tmp/interlogger.perf  --proxy-il-fprefix /tmp/perftest.log"

PERFTEST_CONSUMER="$STAGEDIR/bin/glite-lb-interlogd-perf-empty"
CONSUMER_ARGS="-d -s /tmp/interlogger.perf --file-prefix=/tmp/perftest.log"
export PERFTEST_NAME="proxy_test_5"
echo -n "5)"
run_test proxy $numjobs
print_result
$PERFTEST_COMPONENT -d -p /tmp/proxy.perf -s 1 >/dev/null 2>&1  &
PID=$!
purge_proxy `$LOGJOBS -n $numjobs`
sleep 2
shutdown $PID
rm -f /tmp/perftest.log.*
