// -*- c-basic-offset: 2; related-file-name: "removed.h" -*-
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

#include "removed.h"
#include <boost/bind.hpp>

Removed::Removed(string name,
                 CommonTablePtr table)
  : Element(name, 0, 1),
    _pullCB(0)
{
  // Connect this element's listener to the table for removals
  table->removalListener(boost::bind(&Removed::listener, this, _1));
}


void
Removed::listener(TuplePtr t)
{
  log(Reporting::WORDY, 0, "RemovalListener " + t->toString());
  _removedBuffer.push_back(t);
  if (_pullCB) {
    _pullCB();
    _pullCB = 0;
  }
}


TuplePtr
Removed::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  if (_removedBuffer.size() == 0) { 
    // No removals queued. Take a raincheck
    _pullCB = cb;
    return TuplePtr(); 
  } else {
    TuplePtr retTuple = _removedBuffer.front();
    _removedBuffer.pop_front();
    log(Reporting::WORDY, 0, "Pull returns " + retTuple->toString());
    return retTuple;
  }
}
