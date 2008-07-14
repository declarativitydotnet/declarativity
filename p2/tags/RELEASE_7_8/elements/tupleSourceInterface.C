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

#include "tupleSourceInterface.h"
#include "loop.h"

TupleSourceInterface::TupleSourceInterface(string name)
  : Element(name, 0, 1), _notBlocked(1)
{
}

int TupleSourceInterface::tuple(TuplePtr tp)
{
  if (_notBlocked) {
    delayCB(0, boost::bind(&TupleSourceInterface::send, this, tp), this);
  }
  return _notBlocked;
}

void TupleSourceInterface::send(TuplePtr tp)
{
  _notBlocked = 
    output(0)->push(tp, boost::bind(&TupleSourceInterface::unblock, this));
}

void TupleSourceInterface::unblock()
{
  _notBlocked = 1;
}
