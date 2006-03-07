// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
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
 * DESCRIPTION: Low-level tracing facility.  Use logging elements if
 *   you can!
 */

#ifndef __TRACE_H__
#define __TRACE_H__

#ifndef TRACE_OFF
#include <iostream>
class TraceObj {
private:
  const char *fn;
public:
  TraceObj(const char *s) 
    : fn(s) 
  {
    std::cerr << ">> " << fn << "\n";
  }
  ~TraceObj() 
  { 
    std::cerr << "<< " << fn << "\n";
  }
};

#define TRC(_x) std::cerr << "tr " << __PRETTY_FUNCTION__ << ":\n\t" << _x << "\n"
#define TRC_FN TraceObj _t(__PRETTY_FUNCTION__)
#else
#define TRC_FN
#define TRC(_x)
#endif /* TRACE_OFF */

#ifndef DEBUG_OFF
#include <iostream>
#define DBG(_x) std::cerr << "db " << __PRETTY_FUNCTION__ << ":\n\t" << _x << "\n"
#else
#define DBG(_x) 
#endif /* DEBUG_OFF */

#endif /* __TRACE_H_ */
