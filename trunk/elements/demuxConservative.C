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

DEFINE_ELEMENT_INITS(DemuxConservative, "DemuxConservative");

DemuxConservative::DemuxConservative(string name,
                                     std::vector< ValuePtr > demuxKeys,
                                     unsigned inputFieldNo)
  : Element(name, 1, demuxKeys.size() + 1),
    _push_cb(0),
    _blockedOutput(-1),
    _inputFieldNo(inputFieldNo)
{
  int port = 1;                 // Skip the first one, which is the
                                // default
  for (std::vector<ValuePtr>::iterator i = demuxKeys.begin(); 
       i != demuxKeys.end();
       i++) {
    _portMap.insert(std::make_pair(*i, port++));
  }
}


DemuxConservative::DemuxConservative(TuplePtr tp)
  : Element("", 1, 1)
{
  assert(false);
  /** TODO ADD GENERIC CONSTRUCTOR
  */
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
  assert(p->size() >= _inputFieldNo); // Must at least contain the input
                                      // field number

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
  PortMap::iterator i = _portMap.find((*p)[_inputFieldNo]);
  unsigned o;
  if (i == _portMap.end()) {
    // Ain't got any. use the default
    o = 0;
  } else {
    o = (*i).second;
  }

  // We found our output. It can't be blocked if we're here. Send it
  // with the appropriate callback
  int result =
    Element::output(o)->push(p, boost::bind(&DemuxConservative::unblock,
                                            this, o));

  // If it can take no more
  if (result == 0) {
    // Block
    _blockedOutput = o;
    
    // And push back my input as well
    assert(!_push_cb);
    _push_cb = cb;
    ELEM_INFO("push: Blocking input due to blocked output '"
              << _blockedOutput
              << "'.");
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
    *ostr << "<o" << 0 << "> default";
    
    PortMap::iterator miter = _portMap.begin();
    while (miter != _portMap.end()) {
      *ostr << "|<o"
            << (*miter).second
            << "> "
            << (*miter).first->toString();
      
      miter++;
    }
    *ostr << "}";
  }
  
  // Close the element label
  *ostr << "}\" ];\n";
}


int 
DemuxConservative::output(ValuePtr key)
{
  // Which of the demux keys does it match?
  PortMap::iterator piter = _portMap.find(key);
  if (piter != _portMap.end()) {
    return piter->second;
  }
  //  return 0;
  return -1; // if we don't find anything, return an invalid port!
} 

