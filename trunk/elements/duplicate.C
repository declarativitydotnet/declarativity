// -*- c-basic-offset: 2; related-file-name: "duplicate.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "duplicate.h"

Duplicate::Duplicate(str name, int outputs)
  : Element(name, 1, outputs),
    _push_cb(cbv_null),
    _block_flags(),
    _block_flag_count(0)
{
  // Clean out the block flags
  _block_flags.zsetsize(noutputs());
}

void Duplicate::unblock(int output)
{
  assert((output >= 0) &&
         (output <= noutputs()));
  
  // Unset a blocked output
  if (_block_flags[output]) {
    log(LoggerI::INFO, -1, "unblock");

    _block_flags[output] = false;
    _block_flag_count--;
    assert(_block_flag_count >= 0);
  }

  // If I have a push callback, call it and remove it
  if (_push_cb != cbv_null) {
    log(LoggerI::INFO, -1, "unblock: propagating aggregate unblock");
    _push_cb();
    _push_cb = cbv_null;
  }
}

int Duplicate::push(int port, TupleRef p, cbv cb)
{
  assert(p != 0);
  assert(port == 0);
  assert(p->size() > 0);

  // Can I take more?
  if (_block_flag_count == noutputs()) {
    // Drop it and hold on to the callback if I don't have it already
    if (_push_cb == cbv_null) {
      _push_cb = cb;
    } else {
      log(LoggerI::WARN, -1, "push: Callback overrun");
    }
    log(LoggerI::WARN, -1, "push: Overrun");
    return 0;
  }

  // For every output
  for (int i = 0;
       i < noutputs();
       i++) {
    // Is the output blocked?
    if (_block_flags[i]) {
      // No can do. Skip this output
      log(LoggerI::INFO, -1, "push: Skipped duplication on blocked output");
    } else {
      // Send it with the appropriate callback
      int result = output(i)->push(p, wrap(this, &Duplicate::unblock, i));

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
    assert(_push_cb == cbv_null);
    _push_cb = cb;
    log(LoggerI::WARN, -1, "push: Blocking input");
    return 0;
  } else {
    return 1;
  }
}

