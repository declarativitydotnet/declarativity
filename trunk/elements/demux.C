// -*- c-basic-offset: 2; related-file-name: "demux.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "demux.h"

Demux::Demux(str name,
             ref< vec< ValueRef > > demuxKeys)
  : Element(name, 1, demuxKeys->size()),
    _push_cb(cbv_null),
    _demuxKeys(demuxKeys),
    _block_flags(),
    _block_flag_count(0)
{
  // Clean out the block flags
  _block_flags.zsetsize(noutputs());
}

void Demux::unblock(int output)
{
  assert((output >= 0) &&
         (output <= noutputs()));
  
  // Unset a blocked output
  if (_block_flags[output]) {
    _block_flags[output] = false;
    _block_flag_count--;
    assert(_block_flag_count >= 0);
  }

  // If I have a push callback, call it and remove it
  if (_push_cb != cbv_null) {
    _push_cb();
    _push_cb = cbv_null;
  }
}

int Demux::push(int port, TupleRef p, cbv cb)
{
  assert(p != 0);
  assert(port == 0);
  assert(p->size() > 0);

  // Can I take more?
  if (_block_flag_count == noutputs()) {
    // Drop it and hold on to the callback if I don't have it already
    if (_push_cb == cbv_null) {
      _push_cb = cb;
    }
    log(LoggerI::WARN, -1, "push: Overrun");
    return 0;
  }

  // Extract the first field of the tuple
  ValueRef first = (*p)[0];

  // XXX Slow version for now.  Use a hash table eventually

  // Which of the demux keys does it match?
  for (int i = 0;
       i < noutputs();
       i++) {
    ValueRef key = (*_demuxKeys)[i];

    // The match must be exact.  No type conversions allowed.
    if (key->equals(first)) {
      // We found our output. Is it blocked?
      if (_block_flags[i]) {
        // No can do. Drop the tuple and return 0 if all outputs are
        // blocked
        log(LoggerI::WARN, -1, "push: Matched blocked output");
        return (_block_flag_count == noutputs());
      } else {
        // Send it with the appropriate callback
        int result = output(i)->push(p, wrap(this, &Demux::unblock, i));

        // If it can take no more
        if (result == 0) {
          // update the flags
          _block_flags[i] = true;
          _block_flag_count++;

          return (_block_flag_count == noutputs());
        } else {
          // Not all outputs are blocked so I can keep on truckin'
          return 1;
        }
      }
    }
  }

  // Didn't find any takers.  Just drop the tuple but keep taking more
  log(LoggerI::WARN, -1, "push: Couldn't match tuple to output");
  return 1;
}

