// -*- c-basic-offset: 2; related-file-name: "ddemux.h" -*-
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

#include "ddemux.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_list.h"
#include "oper.h"
#include <boost/bind.hpp>

using namespace opr;

DEFINE_ELEMENT_INITS(DDemux, "DDemux");

DDemux::DDemux(string name,
               std::vector<ValuePtr> keys,
               unsigned inputFieldNo)
  : Element(name, 1, 1),
    _push_cb(0),
    _block_flag_count(0),
    _inputFieldNo(inputFieldNo)
{
  _block_flags.push_back(false);	// The default port block flag
  for (std::vector<ValuePtr>::iterator i = keys.begin(); 
       i != keys.end(); i++) 
    assert(add_output(*i) > 0);
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_List:   Keys.
 * 4. Val_UInt32: Input field number.
 */
DDemux::DDemux(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _push_cb(0),
    _block_flag_count(0),
    _inputFieldNo(0)
{
  _block_flags.push_back(false);	// The default port block flag

  ListPtr keys = Val_List::cast((*args)[3]);
  for (ValPtrList::const_iterator i = keys->begin(); 
       i != keys->end(); i++) 
    assert(add_output(*i) > 0);
  _inputFieldNo = args->size() > 4 ? Val_Int64::cast((*args)[4]) : 0;
}

int 
DDemux::output(ValuePtr key)
{
  // Which of the demux keys does it match?
  PortMap::iterator piter = _port_map.find(key);
  if (piter != _port_map.end()) {
    return piter->second;
  }
  return -1;
} 

/**
 * Add output port key'ed off of 'key' argument.
 * If such a 'key' already exist, then an assertion error
 * will be raised. 
 * RETURN: port allocated to this key.
 */
unsigned DDemux::add_output(ValuePtr key) {
  PortMap::iterator miter = _port_map.find(key);
  if (miter != _port_map.end())
    return miter->second;	// Port with key already allocated

  unsigned port = addOutputPort(); _port_map.insert(std::make_pair(key, port));

  if (port == _block_flags.size()) {
    _block_flags.push_back(false);
  }
  else {
    // Must be set to unblocked, when removed.
    assert(!_block_flags[port]);	
  }
  return port;
}

int DDemux::remove_output(unsigned port) {
  assert(port != 0);	// Never remove the default port

  for (PortMap::iterator miter = _port_map.begin();
       miter != _port_map.end(); miter++) {
    if (miter->second == port) {
      _port_map.erase(miter); 
      remove_block_flag(port);
      return remove_output(port);
    }
  }
  return -1;
}

int DDemux::remove_output(ValuePtr key) {
  PortMap::iterator miter = _port_map.find(key);
  if (miter != _port_map.end())
    return -1;

  int port = miter->second;
  _port_map.erase(miter);
  remove_block_flag(port);
  return deleteOutputPort(port);
}

void DDemux::remove_block_flag(unsigned port) {
  if (_block_flags[port]) {
     _block_flags[port] = false;
     _block_flag_count--;
  }
  if (port + 1 == _block_flags.size()) {
    _block_flags.pop_back();
  }
}

void DDemux::unblock(unsigned output)
{
  assert((output >= 0) &&
         (output <= _block_flags.size()));
  
  // Unset a blocked output
  if (_block_flags[output]) {
    ELEM_INFO("unblock");

    _block_flags[output] = false;
    _block_flag_count--;
    assert(_block_flag_count >= 0);
  }

  // If I have a push callback, call it and remove it
  if (_push_cb && this->Element::output(output) != 0) {
    ELEM_INFO("unblock: propagating aggregate unblock");
    _push_cb();
    _push_cb = 0;
  }
}

int DDemux::push(int port, TuplePtr p, b_cbv cb)
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

  // Extract the first field of the tuple
  ValuePtr key = (*p)[_inputFieldNo];
  PortMap::iterator piter = _port_map.find(key);
  if (piter != _port_map.end()) {
    int i = piter->second;
    // We found our output. Is it blocked?
    if (_block_flags[i]) {
      // No can do. Drop the tuple and return 0 if all outputs are
      // blocked
      TELL_ERROR << "DDemux tuple dropped!: " << p->toString() << std::endl;
      ELEM_WARN("push: Matched blocked output");

      // Of course, our input is not blocked, or we wouldn't be here,
      // yes?
      assert(_block_flag_count < noutputs());
    } else {
      // Send it with the appropriate callback
      int result = Element::output(i)->push(p, boost::bind(&DDemux::unblock, this, i));

      // If it can take no more
      if (result == 0) {
        // update the flags
        _block_flags[i] = true;
        _block_flag_count++;

        // If I just blocked all of my outputs, push back my input
        if (_block_flag_count == noutputs()) {
          assert(!_push_cb);
          _push_cb = cb;
          ELEM_WARN("push: Blocking input");
          return 0;
        } 
      } 
    }
  }
  else {
    // The input matched none of the keys.  
    // Send it to the default output (port 0 always)
    if (_block_flags[0]) {
      // No can do. 
      // Drop the tuple and return 0 if all outputs are blocked
      TELL_ERROR << "DDemux tuple dropped!: " << p->toString() << std::endl;
      ELEM_WARN("push: Default output blocked");
    
      // Of course, our input is not blocked, or we wouldn't be here,
      // yes?
      assert(_block_flag_count < noutputs());
    } else {
      // Send it with the appropriate callback
      int result = Element::output(0)->push(p, boost::bind(&DDemux::unblock, this, 0));
    
      // If it can take no more
      if (result == 0) {
	// update the flags
	_block_flags[0] = true;
	_block_flag_count++;
      
	// If I just blocked all of my outputs, push back my input
	if (_block_flag_count == noutputs()) {
	  assert(_push_cb);
	  _push_cb = cb;
	  ELEM_WARN("push: Blocking input");
	  return 0;
	}
      }
    }
  }
  return 1;
}



void
DDemux::toDot(std::ostream* ostr)
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
    // For demux, put the names of the output in there as well
    *ostr << "|{";
    
    DDemux::PortMap::iterator miter =
      _port_map.begin();
    while (miter != _port_map.end()) {
      *ostr << "<o"
            << miter->second
            << "> "
            << miter->first->toString();
      
      miter++;
      *ostr << "|";
    }
    *ostr << "<o0> default";
    *ostr << "}";
  }
  
  // Close the element label
  *ostr << "}\" ];\n";
}

