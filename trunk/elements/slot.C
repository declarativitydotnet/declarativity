// -*- c-basic-offset: 2; related-file-name: "slot.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "slot.h"

Slot::Slot(str name)
  : Element(name, 1, 1),
    _push_cb(0),
    _pull_cb(0)
{
}


int Slot::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // A slot is always obliged to accept a tuple. If may discard it.
  // It's also in one of two states - ready for more tuples, or not. 
  // The slot transitions from ready to non-ready on receipt of a
  //  tuple, and returns 0. 
  // When it transitions from non-ready to ready it directly calls the
  //  push callback to let the head of push chain know. 


  // One way or another I must accept a push callback if I don't already
  // have one, since either way I'll block my pushes.
  if (_push_cb) {
    // Complain and do nothing
    log(LoggerI::INFO, 0, "push: callback overrun");
  } else {
    // Accept the callback
    _push_cb = cb;
    log(LoggerI::INFO, 0, "push: raincheck");
  }

  // Do I have a tuple?
  if (_t == NULL) {
    // Nope, accept the tuple
    _t = t;
    log(LoggerI::INFO, 0, "push: put accepted");

    // Unblock the puller if one is waiting
    if (_pull_cb) {
      log(LoggerI::INFO, 0, "push: wakeup puller");
      _pull_cb();
      _pull_cb = 0;
    }
  } else {
    // I already have a tuple so the one I just accepted is dropped
    log(LoggerI::INFO, 0, "push: tuple overrun");
  }
  return 0;
}

TuplePtr Slot::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a tuple?
  if (_t != NULL) {
    log(LoggerI::INFO, 0, "pull: will succeed");
    // I do so I will return it.  First, unblock my pusher if one is
    // waiting
    if (_push_cb) {
      log(LoggerI::INFO, 0, "pull: wakeup pusher");
      _push_cb();
      _push_cb = 0;
    }

    TuplePtr t = _t;
    _t.reset();
    return t;
  } else {
    // I don't have a tuple.  Do I have a pull callback already?
    if (!_pull_cb) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pull_cb = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: underrun");
    }
    return TuplePtr();
  }
}
