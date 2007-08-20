/*
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
#include "tupleListener.h"

TupleListener::TupleListener(string name,
                             TupleCallback callback)
  : Element(name, 1, 0),
    _callback(callback)
{
}


int
TupleListener::push(int port, TuplePtr tp, b_cbv cb)
{
  _callback(tp);
  return 1;
}
