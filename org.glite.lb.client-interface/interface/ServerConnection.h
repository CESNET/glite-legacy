#ifndef __EDG_WORKLOAD_LOGGING_CLIENT_SERVERCONNECTION_HPP__
#define __EDG_WORKLOAD_LOGGING_CLIENT_SERVERCONNECTION_HPP__

#ident "$Header$"

/**
 * @file ServerConnection.h
 * @version $Revision$
 */

#include <string.h>
#include <list>

#include "glite/wmsutils/jobid/JobId.h"

#include "glite/lb/Event.h"
#include "glite/lb/JobStatus.h"
#include "glite/lb/consumer.h"

EWL_BEGIN_NAMESPACE

/** Auxiliary class to hold atomic query condition. 
 *
 * This class is used to construct queries to the L&B database. Each
 * query is composed of multiple atomic conditions in the form of
 * <attribute> <predicate> <value>. QueryRecord represents such an
 * atomic condition. 
 */
class QueryRecord {
public:
	friend class ServerConnection;
	friend edg_wll_QueryRec *convertQueryVector(const std::vector<QueryRecord> &in);

	/* IMPORTANT: must match lbapi.h */
	/** Symbolic names of queryable attributes. 
	 *
	 * The queryable attributes correspond to the table columns in
	 * the bookkeeping server database, they relate both to the
	 * event records  and job records.
	 * \see Event::Attr
	 */
	enum Attr {
		UNDEF=0,	/**< Not-defined value, used to terminate lists etc. */
		JOBID,	        /**< Job id. */
		OWNER,	        /**< Job owner (certificate subject). */
		STATUS,	        /**< Current job status code. */
		LOCATION,	/**< Where is the job being processed. */
		DESTINATION,	/**< Destination CE. */
		DONECODE,	/**< Minor done status (OK,fail,cancel). */
		USERTAG,	/**< User tag. */
		TIME,	        /**< Timestamp of the event. */
		LEVEL,	        /**< Logging level. */
		HOST,	        /**< Hostname where the event was generated. */
		SOURCE,	        /**< Source component that sent the event. */
		INSTANCE,	/**< Instance of the source component. */
		EVENT_TYPE,	/**< Event type. */
		CHKPT_TAG,	/**< Checkpoint tag. */
		RESUBMITTED,	/**< Job was resubmitted */
		PARENT,	        /**< Id of the parent job. */
		EXITCODE,	/**< Job system exit code. */
	};

	/** Symbolic names of predicates.
	 *
	 * These are the predicates used for creating atomic query
	 * conditions.
	 */
	enum Op {
		EQUAL=EDG_WLL_QUERY_OP_EQUAL, /**< Equal. */
		LESS=EDG_WLL_QUERY_OP_LESS, /**< Less than. */
		GREATER=EDG_WLL_QUERY_OP_GREATER, /**< Greater than. */
		WITHIN=EDG_WLL_QUERY_OP_WITHIN, /**< Within the
						   range. */
		UNEQUAL=EDG_WLL_QUERY_OP_UNEQUAL /**< Not equal. */
	};
  

	/** Default constructor.
	 *
	 * Initializes empty query condition.
	 */
	QueryRecord();

	/** Copy constructor
	 * 
	 * Initializes an exact copy of the object.
	 * \param[in] src Original object.
	 */
	QueryRecord(const QueryRecord &);

	/** Assignment operator.
	 *
	 * Initializes an exact copy of the object.
	 * \param[in] src Original object.
	 * \returns Reference to this object.
	 */
	QueryRecord& operator=(const QueryRecord &);

	/** Constructor for condition on string typed value.
	 *
	 * Initializes the object to hold condition on string typed
	 * attribute value.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value Actual value.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const std::string &);

	/** Constructor for condition on integer typed value.
	 *
	 * Initializes the object to hold condition on integer typed
	 * attribute value.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value Actual value.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const int);

	/** Constructor for condition on timeval typed value.
	 *
	 * Initializes the object to hold condition on timeval typed
	 * attribute value.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value Actual value.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const struct timeval &);

	/** Constructor for condition on JobId typed value.
	 *
	 * Initializes the object to hold condition on JobId typed
	 * attribute value.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value Actual value.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const glite::wmsutils::jobid::JobId&);

	/* this one is for attr==TIME and particular state */
	/** Constructor for condition on timeval typed value.
	 *
	 * Initializes the object to hold condition on the time the job 
	 * stays in given state.
	 * \param[in] name Name of the attribute.
	 * \param[in] state State of thet job.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value Actual value.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const int, const struct timeval &);
	
	/* constructors for WITHIN operator */
	/** Constructor for condition on string typed interval.
	 *
	 * Initializes the object to hold condition on string typed
	 * attribute interval.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value_min Low interval boundary.
	 * \param[in] value_max High interval boundary.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const std::string &, const std::string &);

	/** Constructor for condition on integer typed interval.
	 *
	 * Initializes the object to hold condition on integer typed
	 * attribute interval.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value_min Low interval boundary.
	 * \param[in] value_max High interval boundary.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const int, const int);

	/** Constructor for condition on timeval typed interval.
	 *
	 * Initializes the object to hold condition on timeval typed
	 * attribute interval.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value_min Low interval boundary.
	 * \param[in] value_max High interval boundary.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const struct timeval &, const struct timeval &);

	/** Constructor for condition on timeval typed interval for
	 * given state.
	 *
	 * Initializes the object to hold condition on the time job
	 * stayed in given state.
	 * \param[in] name Name of the attribute.
	 * \param[in] op Symbolic name of the predicate.
	 * \param[in] value_min Low interval boundary.
	 * \param[in] value_max High interval boundary.
	 * \param[in] state State of thet job.
	 * \throw Exception Invalid value type for given attribute.
	 */
	QueryRecord(const Attr, const Op, const int, const struct timeval &, const struct timeval &);

	/* convenience for user tags */
	/** Convenience constructor for condition on user tags.
	 *
	 * Initializes the object to hold condition on the value of
	 * user tag.
	 * \param[in] tag Name of the tag.
	 * \param[in] op Symbolic namen of the predicate.
	 * \param[in] value Value of the tag.
	 */
	QueryRecord(const std::string &, const Op, const std::string &);

	/** Convenience constructor for condition on user tags.
	 *
	 * Initializes the object to hold condition on the value of
	 * user tag.
	 * \param[in] tag Name of the tag.
	 * \param[in] op Symbolic namen of the predicate.
	 * \param[in] value_min Minimal value of the tag.
	 * \param[in] value_max Maximal value of the tag.
	 * \throws Exception Predicate is not WITHIN.
	 */
	QueryRecord(const std::string &, const Op, const std::string &, const std::string &);
	
	/** Destructor.
	 *
	 * The actual work is done by member destructors.
	 */
	~QueryRecord();
  
	/** Return the string representation of symbolic attribute
	 * name.
	 * \param[in] attr Symbolic attribute name.
	 * \returns Printable attribute name.
	 */
	static const std::string AttrName(const Attr) ;
  
protected:

	/* conversion to C API type */
	operator edg_wll_QueryRec() const;

private:
	Attr	attr;
	Op	oper;
	std::string tag_name;
	int state;
	std::string string_value;
	glite::wmsutils::jobid::JobId jobid_value;
        int     int_value;
        struct timeval timeval_value;
	std::string string_value2;
        int     int_value2;
        struct timeval timeval_value2;
};


/** Supported aggregate operations. */
enum AggOp { AGG_MIN=1, AGG_MAX, AGG_COUNT };


/**
 * Connection to the L&B server.
 * Maintain connection to the server.
 * Implement non job-specific API calls
 */

class ServerConnection {
public:
	friend class Job;

	ServerConnection(void);

	/* DEPRECATED: do not use
	 * connections are now handled automagically inside the implementation
	 */
	ServerConnection(const std::string &);

	/** Open connection to a given server */
	void open(const std::string &);

	/** Close the current connection */
	void close(void);

	/* END DEPRECATED */

	/* set & get parameter methods */

	/* consumer parameter settings */
	void setQueryServer(const std::string&, int);
	void setQueryTimeout(int);

	void setX509Proxy(const std::string&);
	void setX509Cert(const std::string&, const std::string&);

	std::pair<std::string, int> getQueryServer() const;
	int getQueryTimeout() const;

	std::string getX509Proxy() const;
	std::pair<std::string, std::string> getX509Cert() const;

	/* end of set & get */

	virtual ~ServerConnection();


	/* consumer API */

	/** Retrieve the set of single indexed attributes.
	 * outer vector elements correspond to indices
	 * inner vector elements correspond to index columns
	 * if .first of the pair is USERTAG, .second is its name
	 * if .first is TIME, .second is state name
	 * otherwise .second is meaningless (empty string anyway)
	 */
	std::vector<std::vector<std::pair<QueryRecord::Attr,std::string> > >
		getIndexedAttrs(void);

	/** Retrieve hard and soft result set size limit */
	std::pair<int,int> getLimits(void) const;

	/** Set the soft result set size limit */
	void setQueryJobsLimit(int);
	void setQueryEventsLimit(int);
  
	/** Retrieve all events satisfying the query records
	 * @param job_cond, event_cond - vectors of conditions to be satisfied 
	 *  by jobs as a whole or particular events, conditions are ANDed
	 * @param events vector of returned events
	 */
	void queryEvents(const std::vector<QueryRecord>& job_cond,
			 const std::vector<QueryRecord>& event_cond,
			 std::vector<Event>& events) const;
  
	const std::vector<Event> queryEvents(const std::vector<QueryRecord>& job_cond,
					     const std::vector<QueryRecord>& event_cond) const;

	const std::list<Event> queryEventsList(const std::vector<QueryRecord>& job_cond,
				               const std::vector<QueryRecord>& event_cond) const;


	/** The same as queryEvents but return only an aggregate.
	 * @param job_cond, event_cond - vectors of conditions to be satisfied 
	 *  by jobs as a whole or particular events, conditions are ANDed
	 * @param op aggregate operator to apply
	 * @param attr attribute to apply the operation to
	 */
	std::string queryEventsAggregate(const std::vector<QueryRecord>& job_cond,
					 const std::vector<QueryRecord>& event_cond,
					 enum AggOp const op,
					 std::string const attr) const;
  

	/** Retrieve all events satisfying the query records
	 * @param job_cond, event_cond 	IN vectors of vectors of job or event conditions, 
	 * respectively. The inner vectors are logically ANDed, the outer are ORed
	 * (cond1 AND cond2 AND ...) OR (condN AND ...)
	 * @param eventList 	OUT vector of returned events
	 */
	void queryEvents(const std::vector<std::vector<QueryRecord> >& job_cond,
			 const std::vector<std::vector<QueryRecord> >& event_cond,
			 std::vector<Event>& eventList) const;
  
	const std::vector<Event> 
	queryEvents(const std::vector<std::vector<QueryRecord> >& job_cond,
		    const std::vector<std::vector<QueryRecord> >& event_cond) const;

	
	/** Retrieve jobs satisfying the query records, including their states
	 * @param query 	IN vector of Query records that are ANDed to form the query
	 * @param jobList	OUT vector of returned job id's
	 */
  
	void queryJobs(const std::vector<QueryRecord>& query,
		       std::vector<glite::wmsutils::jobid::JobId>& jobList) const;
  
	const std::vector<glite::wmsutils::jobid::JobId>
	queryJobs(const std::vector<QueryRecord>& query) const;
  
	
	/** Retrieve jobs satisfying the query records, including their states
	 * @param query 	IN vector of Query record vectors that are ORed and ANDed to form the query
	 * @param jobList	OUT vector of returned job id's
	 */
   
	void queryJobs(const std::vector<std::vector<QueryRecord> >& query,
		       std::vector<glite::wmsutils::jobid::JobId>& jobList) const;
  
	const std::vector<glite::wmsutils::jobid::JobId>
	queryJobs(const std::vector<std::vector<QueryRecord> >& query) const;

	/** Retrieve jobs satisfying the query records, including status information
	 * @param query 	IN vector of Query records that are ANDed to form the query
	 * @param flags		IN flags
	 * @param states 	OUT vector of returned job states
	 */
	void queryJobStates(const std::vector<QueryRecord>& query, 
			    int flags,
			    std::vector<JobStatus> & states) const;
	const std::vector<JobStatus>  queryJobStates(const std::vector<QueryRecord>& query, 
						     int flags) const;

	const std::list<JobStatus>  queryJobStatesList(const std::vector<QueryRecord>& query,
						     int flags) const;
  
	/** Retrieve jobs satisfying the query records, including status information
	 * @param query 	IN vector of Query records that are anded to form the query
	 * @param flags		IN flags
	 * @param states 	OUT vector of returned job states
	 */
	void queryJobStates(const std::vector<std::vector<QueryRecord> >& query, 
			    int flags,
			    std::vector<JobStatus> & states) const;
	const std::vector<JobStatus>  
	queryJobStates(const std::vector<std::vector<QueryRecord> >& query, 
		       int flags) const;
  
	/** States of all user's jobs.
	 * Convenience wrapper around queryJobs.
	 */
	void userJobStates(std::vector<JobStatus>& stateList) const;
	const std::vector<JobStatus> userJobStates() const;
  
  
	/** JobId's of all user's jobs.
	 * Convenience wrapper around queryJobs.
	 */
	void userJobs(std::vector<glite::wmsutils::jobid::JobId> &) const;
	const std::vector<glite::wmsutils::jobid::JobId> userJobs() const;

	/** Manipulate LB parameters, the same as for edg_wll_Context in C */
	void setParam(edg_wll_ContextParam, int); 
	void setParam(edg_wll_ContextParam, const std::string); 
	void setParam(edg_wll_ContextParam, const struct timeval &); 

	int getParamInt(edg_wll_ContextParam) const;
	std::string getParamString(edg_wll_ContextParam) const;
	struct timeval getParamTime(edg_wll_ContextParam) const;
  
protected:

	edg_wll_Context getContext(void) const;

private:
	edg_wll_Context context;
};

EWL_END_NAMESPACE

#endif
