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

#include <errno.h>
#include "printTime.h"
#include "loop.h"
#include "val_str.h"

DEFINE_ELEMENT_INITS(PrintTime, "PrintTime");

PrintTime::PrintTime(string prefix)
  : Element(prefix, 1, 1),
    _prefix(prefix)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Prefix / Name.
 */
PrintTime::PrintTime(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _prefix(Val_Str::cast((*args)[2]))
{
}

PrintTime::~PrintTime()
{
}

TuplePtr PrintTime::simple_action(TuplePtr p)
{
  boost::posix_time::ptime now_ts;
  
  getTime(now_ts);
  ELEM_OUTPUT("Print[" << _prefix
              << ", "
              << boost::posix_time::to_simple_string(now_ts)
              << "]:  ["
              << p->toString()
              << "]");
  return p;
}
