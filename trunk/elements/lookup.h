// -*- c-basic-offset: 2; related-file-name: "lookupUnique.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached
 * INTEL-LICENSE file.  If you do not find these files, copies can be
 * found by writing to: Intel Research Berkeley, 2150 Shattuck Avenue,
 * Suite 1300, Berkeley, CA, 94704.  Attention: Intel License Inquiry.
 *
 * The lookup element.  It has a single input where a lookup tuple
 * arrives; the inputKeyField determines which field of this lookup
 * tuple will be used to search the table.  The search of the table is
 * performed according to the index (if any) held for the table's
 * lookupIndexField field.  On the output all matches for the lookup are
 * returned with successive pulls.  Outputs consist of tuples that
 * contain two embedded tuples, the first holding the lookup tuple and
 * the second holding the result if any.  The last output tuple to be
 * returned for a given search is tagged with END_OF_SEARCH.  If a
 * lookup yields no results, an output tuple with the lookup tuple as
 * the first value and an empty tuple as the second value is returned
 * (also tagged with END_OF_SEARCH).
 */

#ifndef __LOOKUP_H__
#define __LOOKUP_H__

#include "element.h"
#include "table.h"
#include "val_tuple.h"
#include "val_null.h"

template < typename _EncapsulatedIterator, typename _LookupGenerator >
class Lookup : public Element {
public:
  Lookup(str name,
         TableRef table,
         unsigned inputKeyField,
         unsigned lookupIndexField,
	 cbv completion_cb=cbv_null );
  ~Lookup();


  const char *class_name() const		{ return "Lookup";}
  const char *processing() const		{ return "h/l"; }
  const char *flow_code() const			{ return "-/-"; }
  
  /** Receive a new lookup key */
  int push(int port, TupleRef, cbv cb);
  
  /** Return a match to the current lookup */
  TuplePtr pull(int port, cbv cb);
  
  /** The END_OF_SEARCH tuple tag. */
  static str END_OF_SEARCH;
  
private:
  /** My table */
  TableRef _table;
  
  /** My pusher's callback */
  cbv _pushCallback;
  
  /** My puller's callback */
  cbv _pullCallback;
  
  /** My completion callback */
  cbv _compCallback;
  
  /** My current lookup tuple */
  TuplePtr _lookupTuple;
  
  /** My encapsulated lookup tuple for all results */
  ValuePtr _lookupTupleValue;
  
  /** My current lookup key */
  ValuePtr _key;
  
  /** My current iterator */
  _EncapsulatedIterator _iterator;

  /** My input key field */
  unsigned _inputFieldNo;

  /** My index field */
  unsigned _indexFieldNo;
};


// Define unique and multiple lookups

typedef Lookup< Table::UniqueIterator, Table::UniqueLookupGenerator > UniqueLookup;
typedef Lookup< Table::MultIterator, Table::MultLookupGenerator > MultLookup;



//////////////////////////////////////////////////////////////////////////////////
// VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV//
// What would be in the .C file, if this weren't a template                     //
//////////////////////////////////////////////////////////////////////////////////


template < typename _EncapsulatedIterator, typename _LookupGenerator >
Lookup< _EncapsulatedIterator, _LookupGenerator >::Lookup(str name,
                                                          TableRef table,
                                                          unsigned inputKeyField,
                                                          unsigned lookupIndexField,
							  cbv completion_cb )

  : Element(name, 1, 1),
    _table(table),
    _pushCallback(cbv_null),
    _pullCallback(cbv_null),
    _compCallback(completion_cb),
    _lookupTuple(NULL),
    _lookupTupleValue(NULL),
    _key(NULL),
    _iterator(NULL),
    _inputFieldNo(inputKeyField),
    _indexFieldNo(lookupIndexField)
{
}

template < typename _EncapsulatedIterator, typename _LookupGenerator >
Lookup< _EncapsulatedIterator, _LookupGenerator >::~Lookup()
{
}

template < typename _EncapsulatedIterator, typename _LookupGenerator >
int
Lookup< _EncapsulatedIterator, _LookupGenerator >::push(int port,
                                                        TupleRef t,
                                                        cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a lookup pending?
  if (_lookupTuple == NULL) {
    // No pending lookup.  Take it in
    assert(_pushCallback == cbv_null);
    assert((_key == NULL) && (_lookupTupleValue == NULL) && (_iterator == NULL));

    // Fetch the search field
    _key = (*t)[_inputFieldNo];
    if (_key == NULL) {
      // No input field? WTF?
      log(LoggerI::WARN, 0, "push: tuple without lookup field received");

      // Didn't work out.  Ask for more.
      return 1;
    } else {
      log(LoggerI::WORDY, 0, strbuf("push: key is no longer null"));

      // Establish the lookup
      _lookupTuple = t;
      _lookupTupleValue = Val_Tuple::mk(t);

      // Groovy.  Signal puller that we're ready to give results
      log(LoggerI::INFO, 0, strbuf("push: accepted lookup of key ") << _key->toString());
      
      // Unblock the puller if one is waiting
      if (_pullCallback != cbv_null) {
        log(LoggerI::INFO, 0, "push: wakeup puller");
        _pullCallback();
        _pullCallback = cbv_null;
      }

      // Fetch the iterator
      _iterator = _LookupGenerator::lookup(_table, _indexFieldNo, _key);
      log(LoggerI::WORDY, 0, strbuf("push: iterator no longer null: ") << (int)(void*)_iterator);
      
      // And stop the pusher since we have to wait until the iterator is
      // flushed one way or another
      _pushCallback = cb;
      return 0;
    }
  } else {
    // We already have a lookup pending
    assert(_pushCallback != cbv_null);
    assert(_key != NULL);
    assert(_iterator != NULL);
    assert(_lookupTuple != NULL);
    assert(_lookupTupleValue != NULL);
    log(LoggerI::WARN, 0, "push: lookup overrun");
    return 0;
  }
}

template < typename _EncapsulatedIterator, typename _LookupGenerator >
TuplePtr
Lookup< _EncapsulatedIterator, _LookupGenerator >::pull(int port,
                                                        cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a pending lookup?
  if (_key == NULL) {
    // Nope, no pending lookup.  Deal with underruns.
    assert((_lookupTuple == NULL) && (_lookupTupleValue == NULL));
    assert(_iterator == NULL);

    if (_pullCallback == cbv_null) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: callback underrun");
    }
    if (_compCallback != cbv_null) { _compCallback(); }
    return 0;
  } else {
    // {
    //   // Dump the contents
    //   strbuf dump;
    //   for (std::multimap< ValueRef, TupleRef >::iterator it = _table->begin();
    //        it != _table->end();
    //        ++it)
    //     dump << "  [" << ((*it).first)->toString() << ", " << ((*it).second)->toString() << "]\n";
    //   log(LoggerI::INFO, 0, dump);
    // }
    log(LoggerI::WORDY, 0, strbuf("pull: key is ") << _key->toString()
        << " and iter is " << (int)(void*)_iterator);
    assert(_iterator != NULL);

    // Is the iterator at its end?  This should only happen if a lookup
    // returned no results at all.
    TuplePtr t;
    if (_iterator->done()) {
      // Empty search. Don't try to dereference the iterator.  Just
      // set the result to the empty tuple, to be tagged later
      t = Tuple::EMPTY;
    } else {
      // This lookup has at least one result.
      t = _iterator->next();
    }
    log(LoggerI::WORDY, 0, strbuf("pull: A iterator is ") << (int)(void*)_iterator);
    TupleRef theT = t;

    // Make an unfrozen result tuple containing first the lookup tuple
    // and then the lookup result
    TupleRef newTuple = Tuple::mk();
    newTuple->append(_lookupTupleValue);
    newTuple->append(Val_Tuple::mk(theT));

    // Now, are we done with this search?
    if (_iterator->done()) {
      log(LoggerI::INFO, 0, strbuf("pull: Finished search on ") << _key->toString());
      log(LoggerI::WORDY, 0, strbuf("pull: A iterator is ") << (int)(void*)_iterator);

      // Tag the result tuple
      newTuple->tag(Lookup::END_OF_SEARCH, Val_Null::mk());

      // Clean the lookup state
      _key = NULL;
      log(LoggerI::WORDY, 0, strbuf("push: key is now null: ") << (int)(void*)_key);
      _lookupTuple = NULL;
      _lookupTupleValue = NULL;
      _iterator = NULL;
      log(LoggerI::WORDY, 0, strbuf("push: iterator now is null"));

      // Wake up any pusher
      if (_pushCallback != cbv_null) {
        log(LoggerI::INFO, 0, "pull: wakeup pusher");
        _pushCallback();
        _pushCallback = cbv_null;
      }
    } else {
      // More results to be had.  Don't tag
    }
    newTuple->freeze();
    return newTuple;
  }
}

/** The END_OF_SEARCH tag */
template < typename _EncapsulatedIterator, typename _LookupGenerator >
str Lookup< _EncapsulatedIterator, _LookupGenerator >::END_OF_SEARCH = "Lookup:END_OF_SEARCH";


#endif /* __LOOKUP_H_ */
