// -*- c-basic-offset: 2; related-file-name: "queue.h" -*-
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

#include "queue.h"
#include "tuple.h"
#include<iostream>
#include "loop.h"
#include "val_str.h"
#include "val_uint32.h"
#include "scheduler.h"

DEFINE_ELEMENT_INITS(Queue, "Queue")

Queue::Queue(string name, unsigned int queueSize, string type)
  : Element(name, 1, 1),
    _stateProxy(new IStateful()),
    _pullCB(0),
    _pushCB(0)
{
  assert(queueSize > 0);
  _size = queueSize;
  _type = type;
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_UInt32: Queue size.
 */
Queue::Queue(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _stateProxy(new IStateful()),
    _pullCB(0),
    _pushCB(0),
    _size(Val_UInt32::cast((*args)[3])),
    _type(args->size() > 4 ? Val_Str::cast((*args)[4]) : "basic")
{
}

Queue::~Queue()
{
}

int
Queue::initialize()
{
  Plumber::scheduler()->registerQueue(_stateProxy, _type);
  Plumber::scheduler()->stateful(_stateProxy);
  return 0;
}

/* push. When receive, put to queue. If have pending, wake up.*/
int Queue::push(int port, TuplePtr p, b_cbv cb)
{
  _q.push(p);  
  _stateProxy->state(IStateful::STATEFUL);
  _stateProxy->size(_q.size());


  ELEM_INFO("Just received Push of '"
            << p->toString()
            << "', current queuesize="
            << _q.size());
  if (_pullCB) {
    // is there a pending callback? If so, wake it up
    _pullCB();
    _pullCB = 0;
  } else {
    ELEM_INFO("No pending pull callbacks");
  }

  // have we reached the max size? If so, we have to wait
  if (_q.size() == _size) {
    ELEM_INFO("Queue has reach max size, queuesize=" << _q.size());
    _pushCB = cb;
    return 0;
  }

  return 1;
}


/* pull. When pull, drain the queue. Do nothing if queue is empty but register callback. */
TuplePtr Queue::pull(int port, b_cbv cb)
{
  if (_q.size() == 0) { 
    ELEM_INFO("Queue is empty during pull");
    _pullCB = cb;
    return TuplePtr(); 
  }
  TuplePtr p = _q.front();
  _q.pop();
  _stateProxy->size(_q.size());
  if(_q.size() == 0)
    _stateProxy->state(IStateful::STATELESS);

  if (_pushCB) {
    _pushCB();
    _pushCB = 0;
  }

  ELEM_INFO("Pull succeed " << p->toString() << ", queuesize=" << _q.size());
  return p;
}



