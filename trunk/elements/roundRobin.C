// -*- c-basic-offset: 2; related-file-name: "roundRobin.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "roundRobin.h"

RoundRobin::RoundRobin(str name,
                       int noInputs)
  : Element(name, noInputs, 1),
    _pull_cb(cbv_null),
    _block_flags(),
    _block_flag_count(0),
    _nextInput(0)               // start with input 0
{
  // Clean out the block flags
  _block_flags.zsetsize(ninputs());
}

void RoundRobin::unblock(int input)
{
  assert((input >= 0) &&
         (input <= ninputs()));
  
  // Unset a blocked input
  if (_block_flags[input]) {
    _block_flags[input] = false;
    _block_flag_count--;
    assert(_block_flag_count >= 0);
  }

  // If I have a pull callback, call it and remove it
  if (_pull_cb != cbv_null) {
    _pull_cb();
    _pull_cb = cbv_null;
  }
}

TuplePtr RoundRobin::pull(int port, cbv cb)
{
  assert(port == 0);

  // Can I give more?
  if (_block_flag_count == ninputs()) {
    // Refuse it and hold on to the callback if I don't have it already
    if (_pull_cb == cbv_null) {
      _pull_cb = cb;
    }
    log(LoggerI::WARN, -1, "pull: Underrun");
    return 0;
  }

  // By now, I'd better have no callbacks stored.
  assert(_pull_cb == cbv_null);

  // Fetch the next unblocked input
  for (int i = 0;
       i < ninputs();
       i++) {
    int currentInput = (_nextInput + i) % ninputs();

    // Is this input blocked?
    if (_block_flags[currentInput]) {
      // Oops, can't get this guy. Try again
    } else {
      // Okey dokey, this input is unblocked.  Fetch the result
      TuplePtr p = input(currentInput)->
        pull(wrap(this, &RoundRobin::unblock, currentInput));
      if (p == NULL) {
        // This input just blocked
        _block_flags[currentInput] = true;
        _block_flag_count++;

        // Did we just run out of unblocked inputs?
        if (_block_flag_count == ninputs()) {
          // No more inputs to try.  Fail to return and block output
          _nextInput = (currentInput++) % ninputs();
          _pull_cb = cb;

          return 0;
        } else {
          // No, we still have inputs to try. Keep going around the loop
        }
      } else {
        // Got an input tuple.  Just keep state and return this tuple
        _nextInput = (currentInput++) % ninputs();
        return p;
      }
    }
  }

  // We should never reach this point
  assert(false);

  return 0;
}

