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

#include <errno.h>
#include "printTime.h"
#include "loop.h"

PrintTime::PrintTime(string prefix)
  : Element(prefix, 1, 1),
    _prefix(prefix)
{
}

PrintTime::~PrintTime()
{
}

TuplePtr PrintTime::simple_action(TuplePtr p)
{
  timespec now_ts;
  
  getTime(now_ts);
  warn << "Print[" << _prefix
       << ", "
       << now_ts.tv_sec
       << ", "
       << now_ts.tv_nsec
       << "]:  [" << p->toString() << "]\n";
  return p;
}
