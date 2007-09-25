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
#include "eventLoop.h"

DEFINE_ELEMENT_INITS(CommitBuf, "CommitBuf");

CommitBuf::CommitBuf(string name)
  : Element(name, 1, 1), 
    _action_cl(boost::bind(&CommitBuf::_action, this)),
    _action_queued(false)
{
}

CommitBuf::CommitBuf(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,1),
    _action_cl(boost::bind(&CommitBuf::_action, this)),
    _action_queued(false)
{
}

void CommitBuf::_action()
{
  ELEM_INFO("CommitBuf Flushing "
            << _buffer.size()
            << " Tuples");
  for (TupleSet::iterator it = _buffer.begin(); it != _buffer.end(); it++ ) {
    if (!output(0)->push(*it, 0) == 1) {
      throw Element::Exception("COMMIT BUFFER: down stream element can't handle all tuples!");
    }
  }
  _buffer.clear();
  _action_queued = false;
}

int 
CommitBuf::push(int port, TuplePtr t, b_cbv cb)
{
  _buffer.insert(t);
  if (!_action_queued) {
    _action_queued = true;
    EventLoop::loop()->enqueue_action(boost::bind(&CommitBuf::_action,this));
  }
  return 1;
}
