// -*- c-basic-offset: 2; related-file-name: "ddemux.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "ddemux.h"

DDemux::DDemux(str name, std::vector<ValueRef> keys,
               unsigned inputFieldNo)
  : Element(name, 1, 1),
    _push_cb(b_cbv_null),
    _block_flag_count(0),
    _inputFieldNo(inputFieldNo)
{
  for (std::vector<ValueRef>::iterator i = keys.begin(); 
       i != keys.end(); i++) 
    assert(add_output(*i) > 0);
}

/**
 * Add output port key'ed off of 'key' argument.
 * If such a 'key' already exist, then an assertion error
 * will be raised. 
 * RETURN: port allocated to this key.
 */
int DDemux::add_output(ValueRef key) {
  PortMap::iterator miter = _port_map.find(key);
  assert (miter == _port_map.end());

  int port = -1;
  if (_unusedPorts.size() > 0) {
    port = _unusedPorts.front();
    _unusedPorts.erase(_unusedPorts.begin());
    _block_flags[port] = false;
  }
  else {
    this->Element::add_output();
    port = noutputs() - 1;
    _block_flags.push_back(false);
  }
  _port_map.insert(std::make_pair(key, port));
  return port;
}

void DDemux::remove_output(int port) {
  for (PortMap::iterator miter = _port_map.begin();
       miter != _port_map.end(); miter++)
    if (miter->second == port)
      remove_output(miter->first);
}

void DDemux::remove_output(ValueRef key) {
  PortMap::iterator miter = _port_map.find(key);
  assert (miter != _port_map.end());
  int port = miter->second;
  _port_map.erase(miter);
  _block_flags[port] = false;
  _unusedPorts.push_back(port);
}

void DDemux::unblock(int output)
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
  if (_push_cb != b_cbv_null) {
    log(LoggerI::INFO, -1, "unblock: propagating aggregate unblock");
    _push_cb();
    _push_cb = b_cbv_null;
  }
}

int DDemux::push(int port, TupleRef p, b_cbv cb)
{
  assert(p != 0);
  assert(port == 0);
  assert(p->size() > 0);

  // Can I take more?
  if (_block_flag_count == noutputs()) {
    // Drop it and hold on to the callback if I don't have it already
    if (_push_cb == b_cbv_null) {
      _push_cb = cb;
    } else {
      log(LoggerI::WARN, -1, "push: Callback overrun");
    }
    log(LoggerI::WARN, -1, "push: Overrun");
    return 0;
  }

  // Extract the first field of the tuple
  ValueRef key = (*p)[_inputFieldNo];
  PortMap::iterator piter = _port_map.find(key);
  if (piter != _port_map.end()) {
    int i = piter->second;
    // We found our output. Is it blocked?
    if (_block_flags[i]) {
      // No can do. Drop the tuple and return 0 if all outputs are
      // blocked
      log(LoggerI::WARN, -1, "push: Matched blocked output");

      // Of course, our input is not blocked, or we wouldn't be here,
      // yes?
      assert(_block_flag_count < noutputs());
    } else {
      // Send it with the appropriate callback
      int result = output(i)->push(p, boost::bind(&DDemux::unblock, this, i));

      // If it can take no more
      if (result == 0) {
        // update the flags
        _block_flags[i] = true;
        _block_flag_count++;

        // If I just blocked all of my outputs, push back my input
        if (_block_flag_count == noutputs()) {
          assert(_push_cb == b_cbv_null);
          _push_cb = cb;
          log(LoggerI::WARN, -1, "push: Blocking input");
          return 0;
        } 
      } 
    }
  }
  else {

    // The input matched none of the keys.  
    // Send it to the default output (the first)
    if (_block_flags[0]) {
      // No can do. 
      // Drop the tuple and return 0 if all outputs are blocked
      log(LoggerI::WARN, -1, "push: Default output blocked");
    
      // Of course, our input is not blocked, or we wouldn't be here,
      // yes?
      assert(_block_flag_count < noutputs());
    } else {
      // Send it with the appropriate callback
      int result = output(0)->push(p, boost::bind(&DDemux::unblock, this, 0));
    
      // If it can take no more
      if (result == 0) {
	// update the flags
	_block_flags[0] = true;
	_block_flag_count++;
      
	// If I just blocked all of my outputs, push back my input
	if (_block_flag_count == noutputs()) {
	  assert(_push_cb == b_cbv_null);
	  _push_cb = cb;
	  log(LoggerI::WARN, -1, "push: Blocking input");
	  return 0;
	}
      }
    }
  }
  return 1;
}


