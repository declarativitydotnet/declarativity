// -*- c-basic-offset: 2; related-file-name: "refresh.h" -*-
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

#include "refresh.h"
#include "val_str.h"
#include "plumber.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(Refresh, "Refresh")

Refresh::Refresh(string name,
                 CommonTablePtr table)
  : Element(name, 0, 1),
    _pullCB(0)
{
  // Connect this element's listener to the table for refreshes
  table->refreshListener(boost::bind(&Refresh::listener, this, _1));
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Str:    Table Name.
 */
Refresh::Refresh(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1),
    _pullCB(0)
{
  CommonTablePtr table = Plumber::catalog()->table(Val_Str::cast((*args)[3]));

  // Connect this element's listener to the table for refreshes
  table->refreshListener(boost::bind(&Refresh::listener, this, _1));
}

void
Refresh::listener(TuplePtr t)
{
  ELEM_WORDY("Listener " << t->toString());
  _refreshBuffer.push_back(t);
  if (_pullCB) {
    _pullCB();
    _pullCB = 0;
  }
}


TuplePtr
Refresh::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);
  
  if (_refreshBuffer.size() == 0) { 
    // No refreshes queued. Take a raincheck
    _pullCB = cb;
    return TuplePtr(); 
  } else {
    TuplePtr retTuple = _refreshBuffer.front();
    _refreshBuffer.pop_front();
    ELEM_WORDY("Pull returns " << retTuple->toString());
    return retTuple;
  }
}
