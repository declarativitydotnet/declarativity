// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Single-tuple queue element for P2 (basically, an
 * explicit scheduling point in the dataflow graph)
 */

#include "slot.h"

int Slot::push(int port, TupleRef t, cbv cb)
{
  // A slot is always obliged to accept a tuple. If may discard it.
  // It's also in one of two states - ready for more tuples, or not. 
  // The slot transitions from ready to non-ready on receipt of a
  //  tuple, and returns 0. 
  // When it transitions from non-ready to ready it directly calls the
  //  push callback to let the head of push chain know. 
  // At the same time, when a tuple arrives 

  // 
  // PUSH state:   Ready  Non-ready
  //  Tuple:        NULL   t
  //
  _push_cb = cb;
  if (_t == NULL) {
    _t = t;
    // Now unblock the pull side
    if (_pull_cb) {
      _pull_cb();
      _pull_cb = cbv_null;
    }
  } else {
    warn << "Slot: overrun\n";
  }
  return (_t == NULL) ? 1 : 0;
}

TuplePtr Slot::pull(int port, cbv cb) 
{
  // A slot either has a tuple ready for a pull or not.  
  // If it does, it gets returned and the push state (see above)
  // transitions to ready, hence we invoke the push callback. 
  TuplePtr t = _t;
  _t = NULL;
  // Do we need to call a callback downstream?
  if (_push_cb) {
    _push_cb();
    _push_cb = cbv_null;
  }
  return t;
}
