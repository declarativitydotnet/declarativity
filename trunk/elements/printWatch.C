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
#include "val_tuple.h"
#include <iostream>
#include <errno.h>

PrintWatch::PrintWatch(string prefix, std::set<string> tableNames, 
		       FILE* output)
  : Element(prefix, 1, 1),
    _prefix(prefix), _output(output)
{
  _tableNames = tableNames;  
}

PrintWatch::~PrintWatch()
{
}

TuplePtr PrintWatch::simple_action(TuplePtr p)
{
  if (_tableNames.find((*p)[0]->toString()) == _tableNames.end()) {
    return p; // we don't care about print this
  }

  double bytes = 0;
  for (unsigned int i = 0; i < p->size(); i++) {
    ValuePtr v = (*p)[i];
    if (v->typeName() == "tuple") {
      TuplePtr t = Val_Tuple::cast(v);
      for (unsigned int j = 0; j < t->size(); j++) {
	bytes += (*t)[j]->size();
      }
    } else {
      bytes += (*p)[i]->size();
    }
  }


  timespec now_ts;
  
  if (clock_gettime(CLOCK_REALTIME,&now_ts)) {
    fatal << "clock_gettime:" << strerror(errno) << "\n";
  }
  ostringstream b;
  b << "Print[" << _prefix
    << ", "
    << now_ts.tv_sec
    << ", "
    << now_ts.tv_nsec
    << "]:  [" << (int) bytes << ", " << p->toString() << "]\n";
  
  if (_output != NULL) {
    fprintf(_output, "%s", b.str().c_str()); 
  } else {
    warn << b.str();
  }
  fflush(_output);
  return p;
}
