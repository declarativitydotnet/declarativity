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

#include "printWatch.h"
#include <iostream>

PrintWatch::PrintWatch(str prefix, std::set<str> tableNames)
  : Element(prefix, 1, 1),
    _prefix(prefix)
{
  _tableNames = tableNames;
}

PrintWatch::~PrintWatch()
{
}

TuplePtr PrintWatch::simple_action(TupleRef p)
{
  if (_tableNames.find((*p)[0]->toString()) == _tableNames.end()) {
    return p; // we don't care about print this
  }

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
