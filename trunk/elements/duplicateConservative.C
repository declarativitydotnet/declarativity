// -*- c-basic-offset: 2; related-file-name: "duplicateConservative.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "duplicateConservative.h"

DuplicateConservative::DuplicateConservative(str name, int outputs)
  : Element(name, 1, outputs),
    _push_cb(cbv_null),
    _block_flags(),
    _block_flag_count(0)
{
  // Clean out the block flags
  _block_flags.zsetsize(noutputs());
}

void DuplicateConservative::unblock(int output)
{
  assert((output >= 0) &&
         (output <= noutputs()));
  
  // Unset a blocked output
  if (_block_flags[output]) {
    log(LoggerI::INFO, -1, strbuf("unblock ") << output);

    _block_flags[output] = false;
    _block_flag_count--;
    assert(_block_flag_count >= 0);
  }

  // If I have no more blocked outputs, unblock my pusher
  if (_block_flag_count == 0) {
   log(LoggerI::INFO, -1, str(strbuf() << "unblock: propagating aggregate unblock " << output));
   //assert(_push_cb != cbv_null);
     _push_cb();
    _push_cb = cbv_null;
  }
}

int DuplicateConservative::push(int port, TupleRef p, cbv cb)
{
  assert(p != 0);
  assert(port == 0);

  // Can I take more?
  if (_block_flag_count > 0) {
    // I'm still blocked
    assert(_push_cb != cbv_null);
    log(LoggerI::WARN, -1, "push: Overrun");
    return 0;
  }

  // We're free and clear
  assert(_block_flag_count == 0);
  assert(_push_cb == cbv_null);

  // For every output
  for (int i = 0;
       i < noutputs();
       i++) {
    // Send it with the appropriate callback
    int result = output(i)->push(p, wrap(this, &DuplicateConservative::unblock, i));

    assert(_block_flags[i] == false);

    // If it can take no more
    if (result == 0) {
      // update the flags
      _block_flags[i] = true;
      _block_flag_count++;
    }
  }

  // If I just blocked any of my outputs, push back my input
  if (_block_flag_count > 0) {
    _push_cb = cb;
    log(LoggerI::WARN, -1, "push: Blocking input");
    return 0;
  } else {
    return 1;
  }
}

