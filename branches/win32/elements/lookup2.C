// -*- c-basic-offset: 2; related-file-name: "lookup2.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached
 * LICENSE file.  If you do not find this file, copies can be
 * found by writing to: Intel Research Berkeley, 2150 Shattuck Avenue,
 * Suite 1300, Berkeley, CA, 94704.  Attention: Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 */

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32

#include "lookup2.h"

Lookup2::Lookup2(string name,
                 CommonTablePtr table,
                 CommonTable::Key lookupKey,
                 CommonTable::Key indexKey,
                 b_cbv state_cb)
  : Element(name, 1, 1),
    _table(table),
    _pushCallback(0),
    _pullCallback(0),
    _stateCallback(state_cb),
    _lookupKey(lookupKey),
    _indexKey(indexKey)
{
  // If the two keys are identical, then we need not use projections.
  _project = (_lookupKey != _indexKey);
}


Lookup2::~Lookup2()
{
}


int
Lookup2::push(int port,
              TuplePtr t,
              b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);
  
  // Do I have a lookup pending?
  if (_lookupTuple == NULL) {
    // No pending lookup.  Take it in
    assert(!_pushCallback);
    assert((_lookupTupleValue == NULL) &&
           (_iterator == NULL));
    
    ELEM_WORDY("push: lookup tuple is no longer null");
    
    // Establish the lookup
    _lookupTuple = t;
    _lookupTupleValue = Val_Tuple::mk(t);
    
    // Groovy.  Signal puller that we're ready to give results
    ELEM_INFO("push: accepted lookup for tuple "
              << _lookupTuple->toString());
    
    // Unblock the puller if one is waiting
    if (_pullCallback) {
      ELEM_INFO("push: wakeup puller");
      _pullCallback();
      _pullCallback = 0;
    }
    
    // Fetch the iterator.
    if (_project) {
      _iterator = _table->lookup(_lookupKey, _indexKey, _lookupTuple);
    } else {
      _iterator = _table->lookup(_indexKey, _lookupTuple);
    }
    
    // And stop the pusher since we have to wait until the iterator is
    // flushed one way or another
    _pushCallback = cb;


    return 0;
  } else {
    // We already have a lookup pending
    assert(_pushCallback);
    assert(_iterator != NULL);
    assert(_lookupTuple.get() != NULL);
    assert(_lookupTupleValue.get() != NULL);
    ELEM_WARN("push: lookup overrun");
    return 0;
  }
}


TuplePtr
Lookup2::pull(int port,
              b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);
  
  // Do I have a pending lookup?
  if (_lookupTuple == NULL) {
    // Nope, no pending lookup.  Deal with underruns.
    assert(_lookupTupleValue == NULL);
    assert(_iterator == NULL);
    
    if (!_pullCallback) {
      // Accept the callback
      ELEM_INFO("pull: registered callback");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      ELEM_WARN("pull: callback underrun");
    }
    return TuplePtr();
  } else {
    assert(_iterator);

    // Is the iterator at its end?
    TuplePtr returnTuple;
    if (_iterator->done()) {
      // Return the empty tuple and complete this lookup.
      returnTuple = Tuple::EMPTY;

      ELEM_INFO("pull: Finished search on tuple "
                << _lookupTuple->toString());
      
      // Clean the lookup state
      _lookupTuple.reset();
      _lookupTupleValue.reset();
      _iterator.reset();
      ELEM_WORDY("push: iterator now is null");
      
      // Wake up any pusher
      if (_pushCallback) {
        ELEM_INFO("pull: wakeup pusher");
        _pushCallback();
        _pushCallback = 0;
      }
      // Notify any state listeners
      if (_stateCallback) {
        ELEM_INFO("My state callback was invoked with false.");
        _stateCallback();
      }
    } else {
      // This lookup has at least one result.
      TuplePtr t = _iterator->next();

      // Make a result tuple containing first the lookup tuple and then
      // the lookup result
      returnTuple = Tuple::mk();
      returnTuple->append(_lookupTupleValue);
      returnTuple->append(Val_Tuple::mk(t));

      returnTuple->freeze();
    }
    
    ELEM_WORDY("pull: Lookup returned tuple "
               << returnTuple->toString());
    return returnTuple;
  }
}

