// -*- c-basic-offset: 2; related-file-name: "print.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <async.h>
#include <arpc.h>
#include "queue.h"
#include "tuple.h"
#include<iostream>

Queue::Queue(int queueSize)
  : Element(1, 1), _pullCB(cbv_null), _pushCB(cbv_null)
{
  _size = queueSize;
}

Queue::~Queue()
{
}

/* push. When receive, put to queue. If have pending, wake up.*/
int Queue::push(int port, TupleRef p, cbv cb)
{

  _q.push(p);  

  if (_pullCB != cbv_null) {
    // is there a pending callback? If so, wake it up
    _pullCB();
    _pullCB = cbv_null;
  }

  // have we reached the max size? If so, we have to wait
  if (_q.size() == _size) {
    log(LoggerI::INFO, 0, "Queue: Reach max size\n");
    _pushCB = cb;
    return 0;
  }

  return 1;
}


/* pull. When pull, drain the queue. Do nothing if queue is empty but register callback. */
TuplePtr Queue::pull(int port, cbv cb)
{
  if (_q.size() == 0) { 
    log(LoggerI::INFO, 0, "Queue: Empty\n");
    _pullCB = cb;
    return 0; 
  }
  TuplePtr p = _q.front();
  _q.pop();

  if (_pushCB != cbv_null) {
    _pushCB();
    _pushCB = cbv_null;
  }

  return p;
}



