#ifndef  EDG_WORKLOAD_COMMON_CLIENT_JOBIDEXCEPTIONS_H
#define EDG_WORKLOAD_COMMON_CLIENT_JOBIDEXCEPTIONS_H

/*
 * JobIdExceptions.h
 * Copyright (c) 2001 The European Datagrid Project - IST programme, all rights reserved.
 */
#include "glite/workload/common/utilities/Exceptions.h"

#define COMMON_JOBID_NAMESPACE_BEGIN namespace edg { namespace workload { namespace common {namespace jobid{
COMMON_JOBID_NAMESPACE_BEGIN
/**
 * JobIdException - Exception thrown by JobId Class
 * @ingroup Common
 * @version 0.1
 * @date 15 April 2002
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
class JobIdException : public edg::workload::common::utilities::Exception {
public:
    /**
     * Update all mandatory Exception Information
     */
  JobIdException (const std::string& file,
				    int line,
				    const std::string& method,
				    int code,
				    const std::string& exception_name) ;
};//End CLass  JobIdException

/**
* WrongIdFieldException
* This Exception is thrown when a Job Id syntax error is found
* A valid Job Identification string should be made as follows:
* <LB address>:<LB port>/ <Unique string> */
class  WrongIdException  : public JobIdException {
public:
  /**
  * Constructor
  * @param file - The source file which has generated the Exception
  * @param line - The line number in the source file where the Exception has been thrown
  * @param method - The Name of the method which has thrown the Exception
  * @param code - The Code of the Error raised
  * @param field - The wrong expression catched */
    WrongIdException(const std::string& file,
				       int line,
				       const std::string& method,
				       int code );
}; //End CLass WrongIdException
/**
*  EmptyIdException
* This Exception is thrown when the user tries to get information from a JobId
* which has not been initialized yet, i.e tries to use the get<field name> Methods
*/
class EmptyIdException : public JobIdException {
public:
  /**
  * Constructor
  * @param file - The source file which has generated the Exception
  * @param line - The line number in the source file where the Exception has been thrown
  * @param method - The Name of the method which has thrown the Exception
  * @param code - The Code of the Error raised
  * @param field - The Empty filed requested for */
    EmptyIdException::EmptyIdException(const std::string& file,
				       int line,
				       const std::string& method,
				       int code ,
				       const std::string& field );
}; //End CLass EmptyIdException
} COMMON_NAMESPACE_END }
#endif

