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

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32
#include "printWatch.h"
#include "p2Time.h"
#include "val_tuple.h"
#include <iostream>
#include <errno.h>
#include "val_str.h"
#include "val_list.h"

DEFINE_ELEMENT_INITS(PrintWatch, "PrintWatch")

PrintWatch::PrintWatch(string prefix, std::set<string> tableNames)
  : Element(prefix, 1, 1),
    _prefix(prefix)
{
  _tableNames = tableNames;  
}

PrintWatch::PrintWatch(string prefix, std::vector<string> tableNames)
  : Element(prefix, 1, 1),
    _prefix(prefix)
{
  for (std::vector<string>::iterator iter = tableNames.begin();
       iter != tableNames.end(); iter++) {
    _tableNames.insert(*iter);
  }
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:  Prefix / Name.
 * 2. Val_List: Table names.
 */
PrintWatch::PrintWatch(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1)
{
  ListPtr tableNames = Val_List::cast((*args)[3]);
  for (ValPtrList::const_iterator i = tableNames->begin();
       i != tableNames->end(); i++)
    _tableNames.insert(Val_Str::cast(*i));
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


  boost::posix_time::ptime now_ts;
  
  getTime(now_ts);
  ELEM_OUTPUT("Print["
              << _prefix
              << ", "
              << boost::posix_time::to_simple_string(now_ts)
              << "]:  ["
              << (int) bytes
              << ", "
              << p->toString()
              << "]");
  return p;
}
