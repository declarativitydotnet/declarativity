// -*- c-basic-offset: 2; related-file-name: "store.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Storage component with two element interfaces, one for
 * storing tuples and one for retrieving tuples.
 */

#ifndef __STORE_H__
#define __STORE_H__

#include "element.h"
#include <map>
#include "val_opaque.h"

class Store {

private:
  /** My name */
  str _name;

  /** The primary key of the stored tuples */
  unsigned _fieldNo;

  /** My tupleref comparison functor type for the set. */
  struct tupleRefCompare : public std::less< ValueRef >
  {
    bool operator()(const ValueRef first,
                    const ValueRef second) const
    {
      return first->compareTo(second) < 0;
    }
  };
  
  /** My storage set */
  std::multimap< ValueRef, TupleRef, tupleRefCompare > _table;


 public:

  /** The constructor taking the field number of the primary key */
  Store(str, unsigned);

  /** The insert element.  It has only a single push input on which
      tuples to be inserted into the store are sent.  XXX For now all
      stores succeed. */
  class Insert : public Element {
  public:
    Insert(str name,
           std::multimap< ValueRef, TupleRef, Store::tupleRefCompare > * table,
           unsigned fieldNo);

    const char *class_name() const		{ return "Store::Insert";}
    const char *processing() const		{ return "a/a"; }
    const char *flow_code() const		{ return "x/x"; }

    /** Overridden to perform the insertion operation on passing
        tuples. */
    TuplePtr simple_action(TupleRef p);

  private:
    /** My parent's table */
    std::multimap< ValueRef, TupleRef, Store::tupleRefCompare > * _table;

    /** The field number of the key */
    unsigned _fieldNo;
  };
  
  
  /** The lookup element.  It has a single input where a lookup tuple
      containing only a lookup key arrives.  On the output all matches
      for that lookup key are returned.  This element roughly
      corresponds to an iterator factory.  Every input tuple defines an
      iterator for output tuples.  */
  class Lookup : public Element {
  public:
    Lookup(str name,
           std::multimap< ValueRef, TupleRef, tupleRefCompare > * table);

    const char *class_name() const		{ return "Store::Lookup";}
    const char *processing() const		{ return "h/l"; }
    const char *flow_code() const		{ return "-/-"; }

    /** Receive a new lookup key */
    int push(int port, TupleRef, cbv cb);

    /** Return a match to the current lookup */
    TuplePtr pull(int port, cbv cb);

  private:
    /** My parent's table */
    std::multimap< ValueRef, TupleRef, Store::tupleRefCompare > * _table;
    
    /** My pusher's callback */
    cbv _pushCallback;

    /** My puller's callback */
    cbv _pullCallback;

    /** My current lookup key */
    ValuePtr _key;

    /** My current iterator */
    std::multimap< ValueRef, TupleRef >::iterator _iterator;
  };
  
  
  /** Create a lookup element */
  ref< Lookup > mkLookup() {
    strbuf lName(_name);
    lName.cat(":Lookup");
    return New refcounted< Lookup >(lName, &_table);
  }
  
  /** Create an insert element */
  ref< Insert > mkInsert() {
    strbuf iName(_name);
    iName.cat(":Insert");
    return New refcounted< Insert >(iName, &_table, _fieldNo);
  }

  /** The END_OF_SEARCH tuple tag. */
  static str END_OF_SEARCH;
};


#endif /* __STORE_H_ */
