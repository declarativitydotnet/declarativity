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
#include "val_str.h"
#include "scheduler.h"

DEFINE_ELEMENT_INITS(Delete2, "Delete2");

Delete2::Action::Action(CommonTablePtr tbl) : _table(tbl) {}

void Delete2::Action::commit()
{
  for (TupleSet::iterator i = _buffer.begin(); 
       i != _buffer.end(); i++) {
    _table->remove(*i);
  }
  _buffer.clear();
}

void Delete2::Action::abort()
{
  _buffer.clear();
}

Delete2::Delete2(string name, CommonTablePtr table)
  : Element(name, 1, 0), _action(new Delete2::Action(table))
{
}

Delete2::Delete2(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,0),
    _action(new Delete2::Action(Plumber::catalog()->table(Val_Str::cast((*args)[3]))))
{
}

int 
Delete2::initialize()
{
  Plumber::scheduler()->action(_action);
  return 0;
}

int
Delete2::push(int port, TuplePtr t, b_cbv cb)
{
  _action->addTuple(t);
  return 1;
}
