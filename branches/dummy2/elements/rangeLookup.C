// -*- c-basic-offset: 2; related-file-name: "rangeLookup.h" -*-
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

#include "reporting.h"
#include "rangeLookup.h"
#include "val_str.h"
#include "val_list.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "plumber.h"
#include "scheduler.h"

DEFINE_ELEMENT_INITS(RangeLookup, "RangeLookup")

RangeLookup::RangeLookup(string name,
                 CommonTablePtr table,
                 CommonTable::Key lKey,
		 CommonTable::Key rKey,
                 CommonTable::Key indexKey,
	  	 bool openL,
 	  	 bool openR,
                 b_cbv completion_cb)
  : Element(name, 1, 1),
    _stateProxy(new IStateful()),
    _table(table),
    _pushCallback(0),
    _pullCallback(0),
    _compCallback(completion_cb),
    _indexKey(indexKey),
    _lKey(lKey), _rKey(rKey),
    _openL(openL), _openR(openR)
{
}


RangeLookup::RangeLookup(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]),1,1),
    _stateProxy(new IStateful()),
    _table(Plumber::catalog()->table(Val_Str::cast((*args)[3]))),
    _pushCallback(0),
    _pullCallback(0),
    _compCallback(0)
{
  ListPtr lookupKey = Val_List::cast((*args)[4]);
  for (ValPtrList::const_iterator i = lookupKey->begin();
       i != lookupKey->end(); i++)
    _lKey.push_back(Val_UInt32::cast(*i));

  lookupKey = Val_List::cast((*args)[5]);
  for (ValPtrList::const_iterator i = lookupKey->begin();
       i != lookupKey->end(); i++)
    _rKey.push_back(Val_UInt32::cast(*i));


  ListPtr indexKey = Val_List::cast((*args)[6]);
  for (ValPtrList::const_iterator i = indexKey->begin();
       i != indexKey->end(); i++)
    _indexKey.push_back(Val_UInt32::cast(*i));

  if(Val_Int32::cast((*args)[7]) == 1)
    _openL = true;
  else
    _openL = false;

  if(Val_Int32::cast((*args)[8]) == 1)
    _openL = true;
  else
    _openL = false;

}


RangeLookup::~RangeLookup()
{
}

int
RangeLookup::initialize()
{
  Plumber::scheduler()->stateful(_stateProxy);
  return 0;
}

int
RangeLookup::push(int port,
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

    ELEM_INFO("RangeLookup looking using tuple: "
              << _lookupTuple->toString());
    // Fetch the iterator.
    _iterator = _table->range1DLookup(_lKey,_rKey, _indexKey, 
					_openL, _openR, _lookupTuple);
    _stateProxy->state(IStateful::STATEFUL);
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
RangeLookup::pull(int port,
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
      ELEM_INFO("pull: callback underrun");
    }
    if (_compCallback) {
      _compCallback();
    }
    return TuplePtr();
  } else {
    assert(_iterator);

    // Is the iterator at its end?  This should only happen if a lookup
    // returned no results at all.
    TuplePtr t;
    if (_iterator->done()) {
      // Empty search. Don't try to dereference the iterator.  Just
      // set the result to the empty tuple, to be tagged later
      t = Tuple::EMPTY();
    } else {
      // This lookup has at least one result.
      t = _iterator->next();
    }
    TuplePtr theT = t;
    
    // Make an unfrozen result tuple containing first the lookup tuple
    // and then the lookup result
    TuplePtr newTuple = Tuple::mk();
    newTuple->append(_lookupTupleValue);
    newTuple->append(Val_Tuple::mk(theT));

    // Now, are we done with this search?
    if (_iterator->done()) {
      ELEM_INFO("pull: Finished search on tuple "
                << _lookupTuple->toString());
      
      // Tag the result tuple
      newTuple->tag(RangeLookup::END_OF_SEARCH, Val_Null::mk());
      
      // Clean the lookup state
      _lookupTuple.reset();
      _lookupTupleValue.reset();
      _iterator.reset();
      ELEM_WORDY("push: iterator now is null");

      _stateProxy->state(IStateful::STATELESS);
      
      // Wake up any pusher
      if (_pushCallback) {
        ELEM_INFO("pull: wakeup pusher");
        _pushCallback();
        _pushCallback = 0;
      }
    } else {
      // More results to be had.  Don't tag
    }
    newTuple->freeze();
    ELEM_WORDY("pull: Lookup returned tuple "
               << newTuple->toString());
    return newTuple;
  }
}

/** The END_OF_SEARCH tag */
string RangeLookup::END_OF_SEARCH = "RangeLookup:END_OF_SEARCH";

