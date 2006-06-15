// -*- c-basic-offset: 2; related-file-name: "duplicateConservative.h" -*-
/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "dDuplicateConservative.h"

DDuplicateConservative::DDuplicateConservative(string name, int outputs)
  : Element(name, 1, outputs),
    _push_cb(0),
    _block_flags(),
    _block_flag_count(0)
{
  // Clean out the block flags
  _block_flags.resize(noutputs());
}

void DDuplicateConservative::unblock(unsigned output)
{
  assert((output >= 0) &&
         (output <= noutputs()));
  
  // Unset a blocked output
  if (_block_flags[output]) {
    log(LoggerI::INFO, -1, "unblock output");

    _block_flags[output] = false;
    _block_flag_count--;
    assert(_block_flag_count >= 0);
  }

  // If I have no more blocked outputs, unblock my pusher
  if (_block_flag_count == 0) {
   log(LoggerI::INFO, -1, "unblock: propagating aggregate unblock output");
     _push_cb();
    _push_cb = 0;
  }
}

int DDuplicateConservative::push(int port, TuplePtr p, b_cbv cb)
{
  assert(p != 0);
  assert(port == 0);

  // Can I take more?
  if (_block_flag_count > 0) {
    // I'm still blocked
    assert(_push_cb);
    log(LoggerI::WARN, -1, "push: Overrun");
    return 0;
  }

  // We're free and clear
  assert(_block_flag_count == 0);
  assert(!_push_cb);

  // For every output
  for (unsigned i = 0;
       i < noutputs();
       i++) {
    // Send it with the appropriate callback
    int result = output(i)->push(p, boost::bind(&DDuplicateConservative::unblock, this, i));

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


unsigned DDuplicateConservative::add_output() {

  unsigned port = addOutputPort();

  _block_flags.push_back(false);  
  
  return port;
}

int DDuplicateConservative::remove_output(unsigned port)
{
  return deleteOutputPort(port);
}
