// -*- c-basic-offset: 2; related-file-name: "noNull.h" -*-
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
#include "noNull.h"

NoNull::NoNull(string name)
  : Element(name, 1, 1)
{
}

NoNull::~NoNull()
{
}

TuplePtr NoNull::simple_action(TuplePtr p)
{
  // Only return p if it has size greater than 0
  if (p->size() > 0) {
    return p;
  } else {
    return TuplePtr();
  }
}
