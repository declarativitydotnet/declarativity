// -*- c-basic-offset: 2; related-file-name: "mux.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "mux.h"
#include "async.h"
#include "loop.h"

Mux::Mux(str name,
         int noInputs)
  : Element(name, noInputs, 1),
    _blocked(false),
    _pushCallbacks(),
    _inputTuples(),
    _catchUp(boost::bind(&Mux::catchUp, this)),
    _timeCallback(NULL),
    _callback(boost::bind(&Mux::callback, this))
{
  for (int i = 0;
       i < ninputs();
       i++) {
    _pushCallbacks.push_back(0);
    _inputTuples.push_back(NULL);
  }
}

void Mux::callback()
{
  // Wake up immediately after and push out all pending tuples.  Remain
  // blocked however.
  assert(_timeCallback == NULL);
  _timeCallback = delayCB(0, _catchUp);
}

void Mux::catchUp()
{
  assert(_blocked);
  assert(_timeCallback != NULL);
  _timeCallback = NULL;

  // Go through all pending tuples XXX we always begin from input 0, so
  // we could potentially have starvation
  for (int i = 0; i < ninputs(); i++) {
    if (isUnusedPort(i)) continue;

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
        _inputTuples[i] = NULL;
      }
    }
  }

  // Wunderbar, we have gotten rid of all of our pending tuples.  Go
  // through and wake up everybody
  for (int i = 0;
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


int Mux::push(int port, TupleRef p, b_cbv cb)
{
  assert((port >= 0) && (port < ninputs()));
  assert (!isUnusedPort(port));

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
      log(LoggerI::WARN, -1, strbuf("pull: Overrun on port ") << port);
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

/** Add a new input port, and return the port number */
int Mux::add_input() {
  int port = -1;
  if (_unusedPorts.size() > 0) {
    port = _unusedPorts.front();
    _unusedPorts.erase(_unusedPorts.begin());
    _pushCallbacks[port] = 0;
    _inputTuples[port] = NULL;
  }
  else {
    this->Element::add_input();
    _pushCallbacks.push_back(0);
    _inputTuples.push_back(NULL);
    port = ninputs() - 1;
  }
  return port;
}

/** Remove port (will not affect other port positions) */
void Mux::remove_input(int port) {
  if (isUnusedPort(port)) 
    return;
  _unusedPorts.push_back(port);
}

bool Mux::isUnusedPort(int port) {
  for (std::vector<int>::iterator iter = _unusedPorts.begin(); 
       iter != _unusedPorts.end(); iter++)
    if (*iter == port) return true;
  return false;
}
