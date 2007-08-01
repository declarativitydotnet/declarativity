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
#include "val_str.h"
#include "plumber.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(Removed, "Removed")

Removed::Removed(string name,
                 CommonTablePtr table)
  : Element(name, 0, 1),
    _pullCB(0)
{
  // Connect this element's listener to the table for removals
  table->removalListener(boost::bind(&Removed::listener, this, _1));
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Str:    Table Name.
 */
Removed::Removed(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1),
    _pullCB(0)
{
  CommonTablePtr table = Plumber::catalog()->table(Val_Str::cast((*args)[3]));

  // Connect this element's listener to the table for refreshes
  table->removalListener(boost::bind(&Removed::listener, this, _1));
}

void
Removed::listener(TuplePtr t)
{
  ELEM_WORDY("RemovalListener " << t->toString());
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
    ELEM_WORDY("Pull returns " << retTuple->toString());
    return retTuple;
  }
}
