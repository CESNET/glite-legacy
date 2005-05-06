#!/bin/sh

# XXX: add path to the stage area
PATH=.:$PATH

#set -x

# Binaries
LOGEV=${LOGEV:-glite-lb-logevent}
JOBLOG=${JOBLOG:-glite-lb-job_log}
JOBREG=${JOBREG:-glite-lb-job_reg}
USERJOBS=${USERJOBS:-glite-lb-user_jobs}
JOBSTAT=${JOBSTAT:-glite-lb-job_status}
PURGE=${PURGE:-glite-lb-purge}

# -m host
BKSERVER_HOST=${BKSERVER_HOST:-`hostname -f`:9000}
LBPROXY_STORE_SOCK=${EDG_WL_LBPROXY_STORE_SOCK:-/tmp/lb_proxy_store.sock}
LBPROXY_SERVE_SOCK=${EDG_WL_LBPROXY_SERVE_SOCK:-/tmp/lb_proxy_serve.sock}

STATES="aborted cancelled done ready running scheduled waiting"
LBPROXY_PURGE_STATES="cleared done aborted cancelled"
JOBS_ARRAY_SIZE=10
SAMPLE_JOBS_ARRAY[0]=
SAMPLE_JOBS_STATES[0]=
SAMPLE_JOBS_RESPONSES[0]=

# some defaults
DEBUG=2
LOGFD=${LOGFD:-1}

# timeouts for polling the bkserver
timeout=10
maxtimeout=300

#
# Procedures
#

# print help message
show_help()
{
	echo  "Usage: $0 [OPTIONS] "
	echo  "Options:"
	echo  " -h | --help                   Show this help message."
	echo  " -x | --proxy-sockpath-pref    LBProxy socket path prefix."
	echo  " -j | --jobs-count             Count of test(ed) jobs."
	echo  " -s | --states                 List of states in which could tested jobs fall."
	echo  " -p | --proxy-purge-states     List of states in which LBProxy purges the job."
	echo  ""
	echo  "For proper operation check your grid-proxy-info"
	grid-proxy-info
}

check_exec()
{
	[ $DEBUG -gt 0 ] && [ -n "$2" ] && echo -n -e "$2\t" || echo -n -e "$1\t"
	eval $1
	RV=$?
	[ $DEBUG -gt 0 ] && [ $RV -eq 0 ] && echo "OK" || echo "FAILED"
	return $RV
}

# check for existance of needed executable(s)
check_utils()
{
	check_exec 'JOBREG=`which $JOBREG`' "Checkig $JOBREG utility" || exit 1
	check_exec 'JOBLOG=`which $JOBLOG`' "Checkig $JOBLOG utility" || exit 1
	check_exec 'LOGEV=`which $LOGEV`' "Checkig $LOGEV utility" || exit 1
	check_exec 'USERJOBS=`which $USERJOBS`' "Checkig $USERJOBS utility" || exit 1
	check_exec 'JOBSTAT=`which $JOBSTAT`' "Checkig $JOBSTAT utility" || exit 1
}

log_ev()
{
#	$LOGEV -j $EDG_JOBID -s NetworkServer -e UserTag --name color --value red
	[ $DEBUG -gt 2 ] && echo "$LOGEV -j \"$EDG_JOBID\" -s UserInterface -c \"$EDG_WL_SEQUENCE\" $@"
	EDG_WL_SEQUENCE=`$LOGEV -j $EDG_JOBID -s UserInterface -c $EDG_WL_SEQUENCE "$@"`
	test $? -ne 0 -o -z "$EDG_WL_SEQUENCE" && echo "missing EDG_WL_SEQUENCE from $LOGEV"
}

log_ev_proxy()
{
#	$LOGEV -x -j $EDG_JOBID -s NetworkServer -e UserTag --name color --value red

	[ $DEBUG -gt 2 ] && echo "$LOGEV -x -j \"$EDG_JOBID\" -s UserInterface -c \"$EDG_WL_SEQUENCE\" $@"
	EDG_WL_SEQUENCE=`$LOGEV -x -j $EDG_JOBID -s UserInterface -c $EDG_WL_SEQUENCE "$@"`
	test $? -ne 0 -o -z "$EDG_WL_SEQUENCE" && echo "missing EDG_WL_SEQUENCE from $LOGEV"
}

purge()
{
	[ $DEBUG -gt 2 ] && echo "$PURGE -a 0 -c 0 -n 0 -o 0 $@"
	$PURGE -a 0 -c 0 -n 0 -o 0 "$@"
}

purge_proxy()
{
	[ $DEBUG -gt 2 ] && echo "$PURGE -x -a 0 -c 0 -n 0 -o 0 $@"
	$PURGE -x -a 0 -c 0 -n 0 -o 0 "$@"
}


db_clear_jobs()
{
	[ $DEBUG -gt 0 ] && echo -n -e "Purging test jobs from db\t\t"
	job=0
	while [ $job -lt $JOBS_ARRAY_SIZE ] ; do
		EDG_WL_SEQUENCE="UI=999999:NS=9999999999:WM=999999:BH=9999999999:JSS=999999:LM=999999:LRMS=999999:APP=999999"
#		log_ev_proxy -e Clear --reason=PurgingDB
#		purge_proxy
#		log_ev -e Clear --reason=PurgingDB
#		purge 

		job=$(($job + 1))
	done
	[ $DEBUG -gt 0 ] && echo "OK"
}

# Test thet registers jobs 
# and checks against lbproxy and bkserver
#
test_gen_sample_jobs()
{
	[ $DEBUG -gt 0 ] && echo -n -e "Registering sample jobs\t\t\t"
	job=0
	while [ $job -lt $JOBS_ARRAY_SIZE ] ; do
#		eval `$JOBREG -x -m $BKSERVER_HOST -s UserInterface 2>&1 | tail -n 2`
		TMP=`$JOBREG -x -m $BKSERVER_HOST -s UserInterface 2>&1`
		[ $? -ne 0 ] && echo -e "ERROR\n\t$JOBREG error!"
		eval `echo "$TMP" | tail -n 2`
		test -z "$EDG_JOBID" && echo "test_gen_sample_jobs: $JOBREG failed" && exit 2
		SAMPLE_JOBS_ARRAY[$job]=$EDG_JOBID

		state=`$JOBSTAT $EDG_JOBID 2>&1 | grep "state :" | cut -d " " -f 3 | tr A-Z a-z`
		proxy_state=`$JOBSTAT -x $LBPROXY_SERVE_SOCK $EDG_JOBID 2>&1 | grep "state :" | cut -d " " -f 3 | tr A-Z a-z`
		if test "$state" != "submitted" ; then
			echo -e "ERROR\n\tjob ${SAMPLE_JOBS_ARRAY[$job]} not submitted succesfully!"
		  	exit 1;
		fi
		if test "$state" != "$proxy_state" ; then
			echo -e "ERROR\n\tjob (${SAMPLE_JOBS_ARRAY[$job]}) records on lbproxy and bkserver differs!"
		  	exit 1;
		fi
		SAMPLE_JOBS_STATES[$job]=$state

		job=$(($job + 1))
	done
	[ $DEBUG -gt 0 ] && echo "OK"
	[ $DEBUG -gt 1 ] && {
		job=0
		while [ $job -lt $JOBS_ARRAY_SIZE ] ; do
			echo ${SAMPLE_JOBS_ARRAY[$job]}
			job=$(($job + 1))
		done
	}
}

# Test that logs random set of events (for registered jobs) to lbproxy
# and chcecks the state in lbproxy
# and measures the time it takes the state to propagate to bkserver
#
test_logging_events()
{
	[ $DEBUG -gt 0 ] && echo -n -e "Logging events to the lbproxy\t\t"
	st_count=`echo $STATES | wc -w`
	job=0
	while [ $job -lt $JOBS_ARRAY_SIZE ] ; do
		tmp=`echo $RANDOM % $st_count + 1 | bc`
		state=`echo $STATES | cut -d " " -f $tmp | tr A-Z a-z`

		source glite-lb-$state.sh -x $LBPROXY_STORE_SOCK -m $BKSERVER_HOST -j ${SAMPLE_JOBS_ARRAY[$job]} 2>&1 1>/dev/null
		[ $? -ne 0 ] && echo -e "ERROR\n\tglite-lb-$state.sh ${SAMPLE_JOBS_ARRAY[$job]} error!"
		proxy_state=`$JOBSTAT -x $LBPROXY_SERVE_SOCK ${SAMPLE_JOBS_ARRAY[$job]} 2>&1 | grep "state :" | cut -d " " -f 3 | tr A-Z a-z`
		purged=`echo $LBPROXY_PURGE_STATES | grep $state`
		bkserver_state=`$JOBSTAT ${SAMPLE_JOBS_ARRAY[$job]} 2>&1 | grep "state :" | cut -d " " -f 3 | tr A-Z a-z`

		if test -n "$purged" ; then
			echo $proxy_state | grep "No such file or directory"
			if test $? -eq 0 ; then
				echo -e "ERROR\n\tJob ${SAMPLE_JOBS_ARRAY[$job]} was not purged out from LBProxy!"
		  		exit 1;
			fi
		fi
		if test -z "$purged" ; then
			if test "$state" != "$proxy_state" ; then
				echo -e "ERROR\n\tevents for job ${SAMPLE_JOBS_ARRAY[$job]} were not logged succesfully!"
			  	exit 1;
			fi
		fi
		
		response=0
		while [ "$state" != "$bkserver_state" ] ; do
			bkserver_state=`$JOBSTAT ${SAMPLE_JOBS_ARRAY[$job]} 2>&1 | grep "state :" | cut -d " " -f 3 | tr A-Z a-z`
			[ $DEBUG -gt 0 ] && echo -n "."
			sleep $timeout
			response=$(($response + $timeout ))
			if test $response -gt $maxtimeout ; then
				echo -e "ERROR\n\tstatus of job ${SAMPLE_JOBS_ARRAY[$job]} as queried from bkserver ($bkserver_state) has not become $state for more than $response seconds!"
			  	exit 1;
			fi
		done

		SAMPLE_JOBS_STATES[$job]=$state
		SAMPLE_JOBS_RESPONSES[$job]=$response
		job=$(($job + 1))
	done
	[ $DEBUG -gt 0 ] && echo "OK"
	[ $DEBUG -gt 1 ] && {
		job=0
		echo "Polling the bkserver took for individual jobs the following time"
		while [ $job -lt $JOBS_ARRAY_SIZE ] ; do
			echo -e "${SAMPLE_JOBS_ARRAY[$job]} (${SAMPLE_JOBS_STATES[$job]})\t${SAMPLE_JOBS_RESPONSES[$job]} seconds"
			job=$(($job + 1))
		done
	}
}


#
# shell starting code

# without parameters show help message
# test -z "$1" &&	show_help

while test -n "$1"
do
	case "$1" in
	"-h" | "--help") show_help && exit 0 ;;
	"-x" | "--proxy-sockpath-pref")
		shift
		export LBPROXY_STORE_SOCK=$1store.sock
		export LBPROXY_SERVE_SOCK=$1serve.sock
		;;
	"-m" | "--bkserver") shift ; BKSERVER_HOST=$1 ;;
	"-j" | "--jobs-count") shift; JOBS_ARRAY_SIZE=$1 ;;
	"-s" | "--states") shift; STATES="$1" ;;
	"-p" | "--proxy-purge-states") shift; LBPROXY_PURGE_STATES="$1" ;;
	"-l" | "--log") shift ; logfile=$1 ;;

	*) echo "Unrecognized option $1" ;;

	esac
	shift
done

if test -n "$logfile" ; then
    LOGFD=3
    exec 3>$logfile
fi


echo "STATES = $STATES"
echo "LBPROXY_PURGE_STATES = $LBPROXY_PURGE_STATES"

check_utils

test_gen_sample_jobs
test_logging_events

db_clear_jobs
