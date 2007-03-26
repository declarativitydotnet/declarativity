// -*- c-basic-offset: 2; related-file-name: "demuxConservative.h" -*-
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

#include "demuxConservative.h"
#include <boost/bind.hpp>

DemuxConservative::DemuxConservative(string name,
                                     std::vector< ValuePtr > demuxKeys,
                                     unsigned inputFieldNo)
  : Element(name, 1, demuxKeys.size() + 1),
    _demuxKeys(demuxKeys),
    _push_cb(0),
    _blockedOutput(-1),
    _inputFieldNo(inputFieldNo)
{
}


void
DemuxConservative::unblock(unsigned output)
{
  assert((output >= 0) &&
         (output <= noutputs()));
  
  // Unset a blocked output if it's the one on which I got the unblock
  // callback
  if (_blockedOutput == (int) output) {
    ELEM_INFO("unblock");
    _blockedOutput = -1;

    // If I have a push callback, call it and remove it
    if (_push_cb) {
      ELEM_INFO("unblock: propagating unblock");
      _push_cb();
      _push_cb = 0;
    }
  } else {
    // I seem to have received a spurrious unblock
    ELEM_WARN("Received spurrious unblock callback from output port "
              << output);
  }
}


int
DemuxConservative::push(int port, TuplePtr p, b_cbv cb)
{
  assert(p != 0);
  assert(port == 0);
  assert(p->size() > 0);

  // Can I take more?
  if (_blockedOutput != -1) {
    // Nope, I'm already blocked. Drop it.

    if (!_push_cb) {
      _push_cb = cb;
    } else {
      ELEM_WARN("push: Second push callback registered!");
    }
    ELEM_WARN("push: Overrun");
    return 0;
  }

  // Extract the demux field of the tuple
  ValuePtr first = (*p)[_inputFieldNo];

  // Which of the demux keys does it match?
  for (unsigned i = 0;
       i < noutputs() - 1;
       i++) {
    ValuePtr key = _demuxKeys[i];

    // The match must be exact.  No type conversions allowed.
    if (key->equals(first)) {
      // We found our output. It can't be blocked if we're here
      // Send it with the appropriate callback
      int result =
        output(i)->push(p, boost::bind(&DemuxConservative::unblock, this, i));

      // If it can take no more
      if (result == 0) {
        // Block
        _blockedOutput = i;

        // And push back my input as well
        assert(!_push_cb);
        _push_cb = cb;
        ELEM_INFO("push: Blocking input");
        return 0;
      } else {
        // I can still take more
        return 1;
      }
    }
  }

  // The input matched none of the keys.  Send it to the default output
  // (the last) with the appropriate callback
  int result =
    output(noutputs() - 1)->
    push(p, boost::bind(&DemuxConservative::unblock, this, noutputs() - 1));
    
  // If it can take no more
  if (result == 0) {
    // update the flags
    _blockedOutput = noutputs() - 1;
      
    assert(!_push_cb);
    _push_cb = cb;
    ELEM_INFO("push: Blocking input");
    return 0;
  } else {
    // I can still take more
    return 1;
  }
}


void
DemuxConservative::toDot(std::ostream* ostr)
{
  *ostr << ID()                 // unique element ID
        << " [ label=\"{";
  
  // Now figure out how many input ports
  if (ninputs() > 0) {
    *ostr << "{<i0> 0";
    for (unsigned p = 1;
         p < ninputs();
         p++) {
      *ostr << "| <i" << p << "> " << p << " ";
    }
    *ostr << "}|";
  }
      
  // Show the name
  *ostr << class_name() // the official type
        << "\\n"
        << name();   // the official name
  
  // And figure out the output ports.
  if (noutputs() > 0) {
    *ostr << "|{";
    
    std::vector< ValuePtr >::iterator miter =
      _demuxKeys.begin();
    uint counter = 0;
    while (miter != _demuxKeys.end()) {
      *ostr << "<o"
            << counter
            << "> "
            << (*miter)->toString();
      
      miter++;
      counter++;
      *ostr << "|";
    }
    *ostr << "<o" << counter << "> default";
    *ostr << "}";
  }
  
  // Close the element label
  *ostr << "}\" ];\n";
}


