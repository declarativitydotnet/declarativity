// -*- c-basic-offset: 2; related-file-name: "update.h" -*-
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

#include "update.h"
#include "val_str.h"
#include "plumber.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(Update, "Update")

Update::Update(string name,
               CommonTablePtr table)
  : Element(name, 0, 1),
    _pullCB(0)
{
  // Connect this element's listener to the table for updates
  table->updateListener(boost::bind(&Update::listener, this, _1));
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Str:    Table Name.
 */
Update::Update(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1),
    _pullCB(0)
{
  CommonTablePtr table = Plumber::catalog()->table(Val_Str::cast((*args)[3]));
  if (!table) {
    TELL_ERROR << "Table does not exist! " << (*args)[3]->toString() << std::endl;
    assert(0);
  }

  // Connect this element's listener to the table for updates
  table->updateListener(boost::bind(&Update::listener, this, _1));
}


void
Update::listener(TuplePtr t)
{
  ELEM_WORDY("Listener " << t->toString());
  _updateBuffer.push_back(t);
  if (_pullCB) {
    _pullCB();
    _pullCB = 0;
  }
}


TuplePtr
Update::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  if (_updateBuffer.size() == 0) { 
    // No updates queued. Take a raincheck
    _pullCB = cb;
    return TuplePtr(); 
  } else {
    TuplePtr retTuple = _updateBuffer.front();
    _updateBuffer.pop_front();
    ELEM_WORDY("Pull returns " << retTuple->toString());
    return retTuple;
  }
}
