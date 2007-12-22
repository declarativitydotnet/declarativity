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

#include "commitBuf.h"
#include "commonTable.h"
#include "val_str.h"
#include "plumber.h"
#include "scheduler.h"

DEFINE_ELEMENT_INITS(CommitBuf, "CommitBuf");

CommitBuf::CommitBuf(string name)
  : Element(name, 1, 1), 
    _action(new CommitBuf::Action(boost::bind(&CommitBuf::flush, this, _1))) { }

CommitBuf::CommitBuf(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,1),
    _action(new CommitBuf::Action(boost::bind(&CommitBuf::flush, this, _1))) { }

void CommitBuf::flush(TupleSet *buffer)
{
  ELEM_WORDY("CommitBuf Flushing "
             << buffer->size()
             << " Tuples");
  for (TupleSet::iterator it = buffer->begin(); 
       it!= buffer->end();it++){
    if (!output(0)->push(*it, 0) == 1) {
      throw Element::Exception("COMMIT BUFFER: down stream element can't handle all tuples!");
    }
  }
}

int
CommitBuf::initialize()
{
  Plumber::scheduler()->action(_action);
  return 0;
}

int 
CommitBuf::push(int port, TuplePtr t, b_cbv cb)
{
  _action->addTuple(t);
  return 1;
}

/**********************************
* CommitBuf::Action
*/

CommitBuf::Action::Action(CommitBuf::FlushCB cb) 
  : _flush(cb) {}

void
CommitBuf::Action::commit()
{
  _flush(&_buffer);
  _buffer.clear();

}

void
CommitBuf::Action::abort()
{
  _buffer.clear();
}
