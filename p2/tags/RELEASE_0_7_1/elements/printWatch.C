// -*- c-basic-offset: 2; related-file-name: "printTime.h" -*-
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
 */

#include "printWatch.h"
#include "p2Time.h"
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

PrintWatch::PrintWatch(string prefix, std::vector<string> tableNames, 
		       FILE* output)
  : Element(prefix, 1, 1),
    _prefix(prefix), _output(output)
{
  for (std::vector<string>::iterator iter = tableNames.begin();
       iter != tableNames.end(); iter++) {
    _tableNames.insert(*iter);
  }
}


PrintWatch::~PrintWatch()
{
}

TuplePtr PrintWatch::simple_action(TuplePtr p)
{
/*
  std::cerr << "WATCH TABLES: ";
  for (std::set<string>::iterator iter = _tableNames.begin();
       iter != _tableNames.end(); iter++) 
    std::cerr << *iter << " ";
  std::cerr << std::endl;
*/

  if (_tableNames.find((*p)[0]->toString()) == _tableNames.end()) {
    // std::cerr << "DID NOT FIND TABLE: " << (*p)[0]->toString() << std::endl;
    return p; // we don't care about print this
  }
  // std::cerr << "FOUND WATCH TABLE: " << (*p)[0]->toString() << std::endl;

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


  boost::posix_time::ptime now_ts;
  
  getTime(now_ts);
  ostringstream b;
  b << "Print[" << _prefix
    << ", "
    << boost::posix_time::to_simple_string(now_ts)
    << "]:  [" << (int) bytes << ", " << p->toString() << "]\n";
  
  if (_output != NULL) {
    fprintf(_output, "%s", b.str().c_str()); 
  } else {
    warn << b.str();
  }
  fflush(_output);
  return p;
}
