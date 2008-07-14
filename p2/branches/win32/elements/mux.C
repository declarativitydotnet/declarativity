// -*- c-basic-offset: 2; related-file-name: "mux.h" -*-
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

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32

#include "mux.h"
#include "loop.h"
#include "val_str.h"
#include "val_uint32.h"
#include <boost/bind.hpp>

DEFINE_ELEMENT_INITS(Mux, "Mux");

Mux::Mux(string name,
         int noInputs)
  : Element(name, noInputs, 1),
    _blocked(false),
    _pushCallbacks(),
    _inputTuples(),
    _catchUp(boost::bind(&Mux::catchUp, this)),
    _timeCallback(NULL),
    _callback(boost::bind(&Mux::callback, this))
{
  for (unsigned i = 0;
       i < ninputs();
       i++) {
    _pushCallbacks.push_back(0);
    _inputTuples.push_back(TuplePtr());
  }
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_UInt32: The number of inputs.
 */
Mux::Mux(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), Val_UInt32::cast((*args)[3]), 1),
    _blocked(false),
    _pushCallbacks(),
    _inputTuples(),
    _catchUp(boost::bind(&Mux::catchUp, this)),
    _timeCallback(NULL),
    _callback(boost::bind(&Mux::callback, this))
{
  for (unsigned i = 0;
       i < ninputs();
       i++) {
    _pushCallbacks.push_back(0);
    _inputTuples.push_back(TuplePtr());
  }
}

void Mux::callback()
{
  // Wake up immediately after and push out all pending tuples.  Remain
  // blocked however.
  assert(_timeCallback == NULL);
  _timeCallback = delayCB(0.0, _catchUp, this);
}

void Mux::catchUp()
{
  assert(_blocked);
  assert(_timeCallback != NULL);
  _timeCallback = NULL;

  // Go through all pending tuples
  for (unsigned i = 0; i < ninputs(); i++) {
    if (_inputTuples[i] != NULL) {
      // Found one.  Push it out
      assert(_pushCallbacks[i]);
      int result = output(0)->push(_inputTuples[i], _callback);

      // Did my output block again?
      if (result == 0) {
        // Oh well, don't try to wake up the pusher.
        return;
      } else {
        // Ah, we got this one out, phew...
        _inputTuples[i].reset();
      }
    }
  }

  // Wunderbar, we have gotten rid of all of our pending tuples.  Go
  // through and wake up everybody
  for (unsigned i = 0;
       i < ninputs();
       i++) {
    if (_pushCallbacks[i]) {
      // Found one.  Wake it up
      _pushCallbacks[i]();
      _pushCallbacks[i] = 0;
    }
  }

  // Finally, we can now unblock
  _blocked = false;
}


int Mux::push(int port, TuplePtr p, b_cbv cb)
{
  assert((port >= 0) && (unsigned(port) < ninputs()));

  // Is my output blocked?
  if (_blocked) {
    // Yup.  Is this input's buffer full?
    if (_inputTuples[port] == NULL) {
      // We have room.  We'd better not have a callback for that input
      assert(!_pushCallbacks[port]);

      // Take it
      _inputTuples[port] = p;
      _pushCallbacks[port] = cb;
      return 0;
    } else {
      // We'd better have a callback
      assert(_pushCallbacks[port]);

      // We have already received a buffered tuple from that input.  Bad
      // input!
      ELEM_WARN("pull: Overrun on port");
      return 0;
    }
  } else {
    // I can forward this
    assert(!_pushCallbacks[port] &&
           (_inputTuples[port] == NULL));

    int result = output(0)->push(p, _callback);

    // Did my output block?
    if (result == 0) {
      _blocked = true;
      // Oh well, store the pusher's callback and push it back
      _pushCallbacks[port] = cb;
      return 0;
    } else {
      // No problem, my output wants more.
      return 1;
    }
  }
}


