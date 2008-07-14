// -*- c-basic-offset: 2; related-file-name: "logger.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element that also exports an interface for making log
 * tuples, which it then pushes. 
 *
 */


#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "element.h"
#include "elementRegistry.h"
#include <loggerI.h>
#include "reporting.h"

class Logger : public Element,
               public LoggerI { 
public:
  Logger(string);
  Logger(TuplePtr args);
  
  const char *class_name() const		{ return "Logger"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/h"; }
  
  /** Override this since it's pure virtual in the interface */
  void log( string classname,
	    string instancename,
	    Reporting::Level severity,
	    int errnum,
	    string explanation );

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  static uint64_t seq;
  

  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __LOGGER_H_ */
