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
#include <set>

class Store {

 public:

  /** The constructor taking the field number of the primary key */
  Store(unsigned);

  /** The insert element.  It has only a single push input on which
      tuples to be inserted into the store are sent.  XXX For now all
      stores succeed. */
  class Insert : public Element {
  public:
    Insert(std::multiset< TupleRef > * table);

    const char *class_name() const		{ return "Store::Insert";}
    const char *processing() const		{ return "h/"; }
    const char *flow_code() const		{ return "-/"; }

    /** Overridden since I have no outputs */
    int push(int port, TupleRef, cbv cb);

  private:
    /** My parent's table */
    std::multiset< TupleRef > * _table;
  };
  
  
  /** The lookup element.  It has a single input where a lookup tuple
      containing only a lookup key arrives.  On the output all matches
      for that lookup key are returned.  This element roughly
      corresponds to an iterator factory.  Every input tuple defines an
      iterator for output tuples.  */
  class Lookup : public Element {
  public:
    Lookup(std::multiset< TupleRef > * table);

    const char *class_name() const		{ return "Store::Lookup";}
    const char *processing() const		{ return "h/l"; }
    const char *flow_code() const		{ return "-/-"; }

    /** Receive a new lookup key */
    int push(int port, TupleRef, cbv cb);

    /** Return a match to the current lookup */
    TuplePtr pull(int port, cbv cb);

  private:
    /** My parent's table */
    std::multiset< TupleRef > * _table;
  };
  
  
  /** Create a lookup element */
  ref< Lookup > mkLookup()			{ return New refcounted< Lookup >(&_table); }
  
  /** Create an insert element */
  ref< Insert > mkInsert()			{ return New refcounted< Insert >(&_table); }
  
private:
  /** The primary key of the stored tuples */
  unsigned _fieldNo;

  /** My tupleref comparison functor type for the set. */
  struct tupleRefCompare : std::less< TupleRef >
  {
  private:
    /** The field number to compare against. */
    unsigned _fieldNo;
  public:
    /** Constructor to set the field number */
    tupleRefCompare(unsigned fieldNo) : _fieldNo(fieldNo) {}
    
    bool operator()(const TupleRef first,
                    const TupleRef second) const
    {
      return (*first)[_fieldNo]->compareTo((*second)[_fieldNo]) < 0;
    }
  };

  /** My comparison functor */
  tupleRefCompare _comparator;

  /** My storage set */
  std::multiset< TupleRef > _table;
};


#endif /* __STORE_H_ */
