// -*- c-basic-offset: 2; related-file-name: "demux.h" -*-
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

#include "demux.h"
#include "val_str.h"
#include "val_int64.h"
#include "val_list.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(Demux, "Demux");

Demux::Demux(string name,
             std::vector< ValuePtr > demuxKeys,
             unsigned inputFieldNo)
  : Element(name, 1, demuxKeys.size() + 1),
    _demuxKeys(demuxKeys),
    _push_cb(0),
    _block_flags(),
    _block_flag_count(0),
    _inputFieldNo(inputFieldNo)
{
  // Clean out the block flags
  _block_flags.resize(noutputs());
}


/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_List:   Key values. 
 * 4. Val_Int64: Input field number.
 */
Demux::Demux(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, Val_List::cast((*args)[3])->size()+1),
    _push_cb(0),
    _block_flags(),
    _block_flag_count(0),
    _inputFieldNo(0)
{
  // Clean out the block flags
  _block_flags.resize(noutputs());

  ListPtr keys = Val_List::cast((*args)[3]);
  for (ValPtrList::const_iterator i = keys->begin();
       i != keys->end(); i++)
    _demuxKeys.push_back(*i);
  _inputFieldNo = args->size() > 4 ? Val_Int64::cast((*args)[4]) : 0;
}


void
Demux::unblock(unsigned output)
{
  assert((output >= 0) &&
         (output <= noutputs()));
  
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

int 
Demux::output(ValuePtr key)
{
  // Which of the demux keys does it match?
  for (unsigned i = 0; i < noutputs() - 1; i++) {
    ValuePtr dkey = _demuxKeys[i];

    // The match must be exact.  No type conversions allowed.
    if (dkey->equals(key)) {
      // We found our output
      return i;
    }
  }
  return -1;
} 

int Demux::push(int port, TuplePtr p, b_cbv cb)
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
  ValuePtr first = (*p)[_inputFieldNo];

  // XXX Slow version for now.  Use a hash table eventually

  // Which of the demux keys does it match?
  for (unsigned i = 0;
       i < noutputs() - 1;
       i++) {
    ValuePtr key = _demuxKeys[i];

    // The match must be exact.  No type conversions allowed.
    if (key->equals(first)) {
      // We found our output. Is it blocked?
      if (_block_flags[i]) {
        // No can do. Drop the tuple and return 0 if all outputs are
        // blocked
        ELEM_INFO("push: Matched blocked output");

        // Of course, our input is not blocked, or we wouldn't be here,
        // yes?
        assert(_block_flag_count < noutputs());
        return 1;
      } else {
        // Send it with the appropriate callback
        int result = Element::output(i)->push(p, boost::bind(&Demux::unblock, this, i));

        // If it can take no more
        if (result == 0) {
          // update the flags
          _block_flags[i] = true;
          _block_flag_count++;

          // If I just blocked all of my outputs, push back my input
          if (_block_flag_count == noutputs()) {
            assert(!_push_cb);
            _push_cb = cb;
            ELEM_INFO("push: Blocking input");
            return 0;
          } else {
            // I can still take more
            return 1;
          }
        } else {
          // Not all outputs are blocked so I can keep on truckin'
          return 1;
        }
      }
    }
  }


  // The input matched none of the keys.  Send it to the default output
  // (the last)
  if (_block_flags[noutputs() - 1]) {
    // No can do. Drop the tuple and return 0 if all outputs are
    // blocked
    ELEM_INFO("push: Default output blocked");
    
    // Of course, our input is not blocked, or we wouldn't be here,
    // yes?
    assert(_block_flag_count < noutputs());
    return 1;
  } else {
    // Send it with the appropriate callback
    int result = Element::output(noutputs() - 1)->push(p, boost::bind(&Demux::unblock, this, noutputs() - 1));
    
    // If it can take no more
    if (result == 0) {
      // update the flags
      _block_flags[noutputs() - 1] = true;
      _block_flag_count++;
      
      // If I just blocked all of my outputs, push back my input
      if (_block_flag_count == noutputs()) {
        assert(!_push_cb);
        _push_cb = cb;
        ELEM_INFO("push: Blocking input");
        return 0;
      } else {
        // I can still take more
        return 1;
      }
    } else {
      // Not all outputs are blocked so I can keep on truckin'
      return 1;
    }
  }
}


void
Demux::toDot(std::ostream* ostr)
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


