#ifndef __EDG_WORKLOAD_LOGGING_CLIENT_JOB_HPP__
#define __EDG_WORKLOAD_LOGGING_CLIENT_JOB_HPP__

#ident "$Header$"

#include "glite/wmsutils/jobid/JobId.h"

#include "glite/lb/Event.h"
#include "glite/lb/JobStatus.h"
#include "glite/lb/ServerConnection.h"


/**
 * @file Job.h
 * @version $Revision$
 */

EWL_BEGIN_NAMESPACE

/** L&B job.
 *
 * Implementation of L&B job-specific calls.
 * Connection to the server is maintained transparently.
 */
  
class Job {
public:
  Job(void);
  Job(const glite::wmsutils::jobid::JobId &);
  ~Job();
  
  /** Assign new JobId to an existing instance.
   * Connection to server is preserved if possible.
   */
  
  Job & operator= (const glite::wmsutils::jobid::JobId &);

/**
 * Status retrieval bitmasks. Used ORed as Job::status() argument,
 * determine which status fields are actually retrieved.
 */
  static const int STAT_CLASSADS;       /**< various job description fields */
  static const int STAT_CHILDREN;       /**< list of subjob JobId's */
  static const int STAT_CHILDSTAT;      /**< apply the flags recursively to subjobs */

  /** Return job status */
  JobStatus status(int) const;
  
  /** Return all events corresponding to this job */
  void log(std::vector<Event> &) const;
  const std::vector<Event> log(void) const;
  
  /** Return last known address of a listener associated to the job.
   * \param name 	IN name of the listener
   * \return hostname and port number
   */
  const std::pair<std::string,uint16_t> queryListener(const std::string & name) const;
  
  /** 
   * Manipulate LB parameters. 
   *
   * The same as for edg_wll_Context in C 
   *
   * \param ctx 	INOUT context to work with
   * \param val 	IN value
   */
  void setParam(edg_wll_ContextParam ctx, int val); 
  /** 
   * Manipulate LB parameters. 
   *
   * The same as for edg_wll_Context in C 
   *
   * \param ctx 	INOUT context to work with
   * \param val 	IN value
   */
  void setParam(edg_wll_ContextParam ctx, const std::string val); 
  /** 
   * Manipulate LB parameters. 
   *
   * The same as for edg_wll_Context in C 
   *
   * \param ctx 	INOUT context to work with
   * \param val 	IN value
   */
  void setParam(edg_wll_ContextParam ctx, const struct timeval &val); 

  /** 
   * Get LB parameters. 
   *
   * The same as for edg_wll_Context in C 
   *
   * \param ctx 	INOUT context to work with
   * \return integer value of the parameter
   */
  int getParamInt(edg_wll_ContextParam ctx) const;
  /** 
   * Get LB parameters. 
   *
   * The same as for edg_wll_Context in C 
   *
   * \param ctx 	INOUT context to work with
   * \return string value of the parameter
   */
  std::string getParamString(edg_wll_ContextParam ctx) const;
  /** 
   * Get LB parameters. 
   *
   * The same as for edg_wll_Context in C 
   *
   * \param ctx		INOUT context to work with
   * \return timeval value of the parameter
   */
  struct timeval getParamTime(edg_wll_ContextParam ctx) const;
  
private:
  ServerConnection	server;
  glite::wmsutils::jobid::JobId			jobId;
};

EWL_END_NAMESPACE

#endif
