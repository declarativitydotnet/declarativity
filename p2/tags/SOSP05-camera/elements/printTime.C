// -*- c-basic-offset: 2; related-file-name: "printTime.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "printTime.h"

PrintTime::PrintTime(str prefix)
  : Element(prefix, 1, 1),
    _prefix(prefix)
{
}

PrintTime::~PrintTime()
{
}

TuplePtr PrintTime::simple_action(TupleRef p)
{
  timespec now_ts;
  
  if (clock_gettime(CLOCK_REALTIME,&now_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  warn << "Print[" << _prefix
       << ", "
       << now_ts.tv_sec
       << ", "
       << now_ts.tv_nsec
       << "]:  [" << p->toString() << "]\n";
  return p;
}
