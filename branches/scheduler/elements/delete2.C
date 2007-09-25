// -*- c-basic-offset: 2; related-file-name: "delete.h" -*-
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

#include "delete2.h"
#include "plumber.h"
#include "eventLoop.h"
#include "val_str.h"

DEFINE_ELEMENT_INITS(Delete2, "Delete2");

Delete2::Delete2(string name, CommonTablePtr table)
  : Element(name, 1, 0), 
    _table(table),
    _action_cl(boost::bind(&Delete2::_action, this)),
    _action_queued(false)
{
}

Delete2::Delete2(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,0),
    _table(Plumber::catalog()->table((*args)[3]->toString())),
    _action_cl(boost::bind(&Delete2::_action, this)),
    _action_queued(false)
{
}

void Delete2::_action()
{
  for (TupleSet::iterator i = _buffer.begin(); i != _buffer.end(); i++) {
    _table->remove(*i);
  }
  _buffer.clear();
  _action_queued = false;
}

int Delete2::push(int port, TuplePtr t, b_cbv cb)
{
  _buffer.insert(t);
  if (!_action_queued) {
    _action_queued = true;
    EventLoop::loop()->enqueue_action(_action_cl);
  }
  return 1;
}
