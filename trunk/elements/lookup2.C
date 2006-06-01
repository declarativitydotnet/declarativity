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

#include "lookup2.h"

Lookup2::Lookup2(string name,
                 Table2Ptr table,
                 Table2::Key lookupKey,
                 Table2::Key indexKey,
                 b_cbv completion_cb)
  : Element(name, 1, 1),
    _table(table),
    _pushCallback(0),
    _pullCallback(0),
    _compCallback(completion_cb),
    _lookupKey(lookupKey),
    _indexKey(indexKey)
{
  // If the two keys are identical, then we need not use projections.
  _project = (_lookupKey == _indexKey);
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
    
    log(LoggerI::WORDY, 0, "push: lookup tuple is no longer null");
    
    // Establish the lookup
    _lookupTuple = t;
    _lookupTupleValue = Val_Tuple::mk(t);
    
    // Groovy.  Signal puller that we're ready to give results
    log(LoggerI::INFO, 0, "push: accepted lookup for tuple "+ _lookupTuple->toString());
    
    // Unblock the puller if one is waiting
    if (_pullCallback) {
      log(LoggerI::INFO, 0, "push: wakeup puller");
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
    log(LoggerI::WARN, 0, "push: lookup overrun");
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
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: callback underrun");
    }
    if (_compCallback) {
      _compCallback();
    }
    return TuplePtr();
  } else {
    // {
    //   // Dump the contents
    //   strbuf dump;
    //   for (std::multimap< ValuePtr, TuplePtr >::iterator it = _table->begin();
    //        it != _table->end();
    //        ++it)
    //     dump << "  [" << ((*it).first)->toString() << ", " << ((*it).second)->toString() << "]\n";
    //   log(LoggerI::INFO, 0, dump);
    // }
    assert(_iterator);

    // Is the iterator at its end?  This should only happen if a lookup
    // returned no results at all.
    TuplePtr t;
    if (_iterator->done()) {
      // Empty search. Don't try to dereference the iterator.  Just
      // set the result to the empty tuple, to be tagged later
      /*
        std::cerr << "\tNO MORE TUPLES IN ITERATOR" << " IN TABLE " << _table->name << " iterator " << _iterator 
        << " TABLE SIZE = " << _table->size() 
        << " TABLE ADDRESS " << _table << std::endl;
      */
      t = Tuple::EMPTY;
    } else {
      // This lookup has at least one result.
      t = _iterator->next();
      /*
        std::cerr << "\tLOOKUP KEY " << _key->toString() << " IN TABLE " << _table->name << " iterator " << _iterator
        << " RETURN TUPLE " << t->toString() << " TABLE SIZE = " << _table->size() 
        << " TABLE ADDRESS " << _table << std::endl;
      */
    }
    TuplePtr theT = t;
    
    // Make an unfrozen result tuple containing first the lookup tuple
    // and then the lookup result
    TuplePtr newTuple = Tuple::mk();
    newTuple->append(_lookupTupleValue);
    newTuple->append(Val_Tuple::mk(theT));

    // Now, are we done with this search?
    if (_iterator->done()) {
      log(LoggerI::INFO, 0, "pull: Finished search on tuple "
          + _lookupTuple->toString());
      
      // Tag the result tuple
      newTuple->tag(Lookup2::END_OF_SEARCH, Val_Null::mk());
      
      // Clean the lookup state
      _lookupTuple.reset();
      _lookupTupleValue.reset();
      _iterator.reset();
      log(LoggerI::WORDY, 0, "push: iterator now is null");
      
      // Wake up any pusher
      if (_pushCallback) {
        log(LoggerI::INFO, 0, "pull: wakeup pusher");
        _pushCallback();
        _pushCallback = 0;
      }
    } else {
      // More results to be had.  Don't tag
    }
    newTuple->freeze();
    return newTuple;
  }
}

/** The END_OF_SEARCH tag */
string Lookup2::END_OF_SEARCH = "Lookup2:END_OF_SEARCH";

