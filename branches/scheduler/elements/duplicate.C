// -*- c-basic-offset: 2; related-file-name: "duplicate.h" -*-
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

#include "duplicate.h"
#include "val_str.h"
#include "val_int64.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(Duplicate, "Duplicate")

Duplicate::Duplicate(string name, int outputs)
  : Element(name, 1, outputs),
    _push_cb(0),
    _block_flags(),
    _block_flag_count(0)
{
  // Clean out the block flags
  _block_flags.resize(noutputs());
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_Int64: Number of outputs.
 */
Duplicate::Duplicate(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, Val_Int64::cast((*args)[3]))
{
  // Clean out the block flags
  _block_flags.resize(noutputs());
}

void Duplicate::unblock(unsigned output)
{
  assert(output <= noutputs());
  
  // Unset a blocked output
  if (_block_flags[output]) {
    ELEM_INFO("unblock");

    _block_flags[output] = false;
    _block_flag_count--;
    assert(_block_flag_count >= 0);
  }

  // If I have a push callback, call it and remove it
  if (_push_cb) {
    ELEM_INFO("unblock: propagating aggregate unblock");
    _push_cb();
    _push_cb = 0;
  }
}

int Duplicate::push(int port, TuplePtr p, b_cbv cb)
{
  assert(p != 0);
  assert(port == 0);
  assert(p->size() > 0);

  // Can I take more?
  if (_block_flag_count == noutputs()) {
    // Drop it and hold on to the callback if I don't have it already
    if (!_push_cb) {
      _push_cb = cb;
    } else {
      ELEM_WARN("push: Callback overrun");
    }
    ELEM_WARN("push: Overrun");
    return 0;
  }

  // For every output
  for (unsigned i = 0;
       i < noutputs();
       i++) {
    // Is the output blocked?
    if (_block_flags[i]) {
      // No can do. Skip this output
      ELEM_INFO("push: Skipped duplication on blocked output ");
    } else {
      // Send it with the appropriate callback
      int result = output(i)->push(p, boost::bind(&Duplicate::unblock, this, i));

      // If it can take no more
      if (result == 0) {
        // update the flags
        _block_flags[i] = true;
        _block_flag_count++;
      }
    }
  }

  // If I just blocked all of my outputs, push back my input
  if (_block_flag_count == noutputs()) {
    assert(!_push_cb);
    _push_cb = cb;
    ELEM_WARN("push: Blocking input");
    return 0;
  } else {
    return 1;
  }
}

