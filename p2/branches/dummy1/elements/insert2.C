// -*- c-basic-offset: 2; related-file-name: "insert.h" -*-
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

#include "insert2.h"
#include "commonTable.h"
#include "val_str.h"
#include "plumber.h"
#include "scheduler.h"

DEFINE_ELEMENT_INITS(Insert2, "Insert2");

//get hold of the table to perform insertion
Insert2::Action::Action(CommonTablePtr table)
  : _table(table) { }

//actually commits stuff.
//assuming _table->insert is atomic, though.
void 
Insert2::Action::commit()
{
  for (TupleSet::iterator i = _buffer.begin();
       i !=_buffer.end(); i++) {
    _table->insert(*i);
  }
  _buffer.clear();
}

void 
Insert2::Action::abort()
{
  _buffer.clear();
}

Insert2::Insert2(string name, CommonTablePtr table)
  : Element(name, 1, 0), _action(new Action(table))
{
}

Insert2::Insert2(TuplePtr args)
  : Element (Val_Str::cast((*args)[2]), 1,0)
{
  CommonTablePtr tbl = Plumber::catalog()->table((*args)[3]->toString());
  _action.reset(new Action(tbl));
}

int 
Insert2::initialize()
{
  Plumber::scheduler()->action(_action);
  return 0;
}

int
Insert2::push(int port, TuplePtr p, b_cbv cb)
{
  _action->addTuple(p);
  return 1;  
}

