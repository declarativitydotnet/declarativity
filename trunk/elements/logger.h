// -*- c-basic-offset: 2; related-file-name: "logger.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that also exports an interface for making log
 * tuples, which it then pushes. 
 *
 */


#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <async.h>
#include <element.h>

class Logger : public Element { 
public:
  
  enum Level { INFO, WARN, ERROR };
  
  // Initialized with the interval between tuple generation events.
  Logger();
  
  const char *class_name() const		{ return "Logger"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/h"; }
  
  // Create a log tuple
  void log( str classname,
	    str instancename,
	    Level severity,
	    int errnum,
	    str explanation );

private:
  static uint64_t seq;
  
};

#endif /* __LOGGER_H_ */
