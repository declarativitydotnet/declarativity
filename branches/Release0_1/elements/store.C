// -*- c-basic-offset: 2; related-file-name: "store.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <utility>
#include "store.h"
#include "val_null.h"

Store::Store(string name,
             unsigned fieldNo)
  : _name(name),
    _fieldNo(fieldNo),
    _table()
{
}


Store::Insert::Insert(string name,
                      std::multimap< ValuePtr, TuplePtr, Store::tuplePtrCompare > * table,
                      unsigned fieldNo)
  : Element(name, 1, 1),
    _table(table),
    _fieldNo(fieldNo)
{
}

Store::Lookup::Lookup(string name,
                      std::multimap< ValuePtr, TuplePtr, Store::tuplePtrCompare > * table)
  : Element(name, 1, 1),
    _table(table),
    _pushCallback(0),
    _pullCallback(0)
{
}

Store::Scan::Scan(string name,
                  std::multimap< ValuePtr, TuplePtr, Store::tuplePtrCompare > * table)
  : Element(name, 0, 1),
    _table(table),
    _iterator(table->end())
{
}

TuplePtr Store::Insert::simple_action(TuplePtr p)
{
  // Fetch the key field
  ValuePtr key = (*p)[_fieldNo];
  if (key == NULL) {
    // No key field? WTF?
    log(LoggerI::WARN, 0, "push: tuple without key field received");
    return TuplePtr();
  } else {
    // Groovy.  Insert it
    _table->insert(std::make_pair(key, p));
    
    log(LoggerI::INFO, 0, "push: accepted tuple");
    return p;
  }
}

void Store::insert(TuplePtr p)
{
  // Fetch the key field
  ValuePtr key = (*p)[_fieldNo];
  if (key == NULL) {
    // No key field? WTF?
    warn << "Store::insert: tuple without key field received";
    return;
  } else {
    // Groovy.  Insert it
    _table.insert(std::make_pair(key, p));
  }
}

int Store::Lookup::push(int port, TuplePtr t, b_cbv cb)
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a lookup pending?
  if (_key == NULL) {
    // No pending lookup.  Take it in
    assert(!_pushCallback);

    // Fetch the first field
    _key = (*t)[0];
    if (_key == NULL) {
      // No first field? WTF?
      log(LoggerI::WARN, 0, "push: tuple without first field received");

      // Didn't work out.  Ask for more.
      return 1;
    } else {
      // Groovy.  Signal puller that we're ready to give results
      string logLine("push: accepted lookup of key ");
      logLine.append(_key->toString());
      log(LoggerI::INFO, 0, logLine);
      
      // Unblock the puller if one is waiting
      if (_pullCallback) {
        log(LoggerI::INFO, 0, "push: wakeup puller");
        _pullCallback();
        _pullCallback = 0;
      }

      // Start the iterator
      _iterator = _table->lower_bound(_key);
      
      // And stop the pusher since we have to wait until the iterator is
      // flushed
      _pushCallback = cb;
      return 0;
    }
  } else {
    // I'm psarry. I can't help ya.
    assert(_pushCallback);
    log(LoggerI::WARN, 0, "push: lookup overrun");
    return 0;
  }
}

TuplePtr Store::Lookup::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have a pending lookup?
  if (_key == NULL) {
    // Nope, no pending lookup.  Deal with underruns.

    if (!_pullCallback) {
      // Accept the callback
      log(LoggerI::INFO, 0, "pull: raincheck");
      _pullCallback = cb;
    } else {
      // I already have a pull callback
      log(LoggerI::INFO, 0, "pull: callback underrun");
    }
    return TuplePtr();
  } else {
    TuplePtr t;
    std::multimap< ValuePtr, TuplePtr >::iterator theEnd = _table->upper_bound(_key);

    // {
    //   // Dump the contents
    //   strbuf dump;
    //   for (std::multimap< ValuePtr, TuplePtr >::iterator it = _table->begin();
    //        it != _table->end();
    //        ++it)
    //     dump << "  [" << ((*it).first)->toString() << ", " << ((*it).second)->toString() << "]\n";
    //   log(LoggerI::INFO, 0, dump);
    // }


    // Is the iterator at its end?  This should only happen if a lookup
    // returned no results at all.
    if (_iterator == theEnd) {
      // Empty search. Don't try to dereference the iterator.  Just
      // set it to the empty tuple, to be tagged later
      t = Tuple::EMPTY;
    } else {
      // This lookup has at least one result.  Fetch a result tuple
      t = _iterator->second;
      _iterator++;
    }
    TuplePtr theT = t;

    // Now, are we done with this search?  XXX For cleanliness, the same
    // condition as above is repeated.  Hopefully the compiler can
    // optimize this
    if (_iterator == theEnd) {
      string logLine("pull: Finished search on ");
      logLine.append(_key->toString());
      log(LoggerI::INFO, 0, logLine);

      // Make a thawed tuple to be tagged
      TuplePtr newTuple = Tuple::mk();
      newTuple->concat(theT);
      newTuple->tag(Store::END_OF_SEARCH, Val_Null::mk());
      newTuple->freeze();

      // Clean the lookup state
      _key.reset();

      // Wake up any pusher
      if (_pushCallback) {
        log(LoggerI::INFO, 0, "pull: wakeup pusher");
        _pushCallback();
        _pushCallback = 0;
      }
      return newTuple;
    } else {
      // More results to be had.  Just return this
      return theT;
    }
  }
}

/** The END_OF_SEARCH tag */
string Store::END_OF_SEARCH = "Store:END_OF_SEARCH";

TuplePtr Store::Scan::pull(int port, b_cbv cb) 
{
  // Is this the right port?
  assert(port == 0);

  // Do I have elements?
  if (_table->size() > 0) {
    // Yes. Is the iterator at the end?
    if (_iterator == _table->end()) {
      // Reset it to the beginning
      _iterator = _table->begin();
    }

    // Get the current element
    TuplePtr t = _iterator->second;
    
    // And advance the iterator.  Don't reset it; it will be done in the
    // next iteration
    _iterator++;

    return t;
  } else {
    // No elements.  Just return the empty tuple
    return Tuple::EMPTY;
  }
}

