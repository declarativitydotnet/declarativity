// -*- c-basic-offset: 2; related-file-name: "tuple.C" -*-
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
 * DESCRIPTION:
 *
 */

#ifndef __TUPLE_H__
#define __TUPLE_H__

#include <boost/shared_ptr.hpp>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <deque>

#include <assert.h>
#include "inlines.h"
#include "value.h"

class Tuple;
typedef boost::shared_ptr<Tuple> TuplePtr;
typedef std::deque< TuplePtr > TuplePtrList;

class Tuple
{

private:
  /** A vector of the fields of this tuple */
  std::vector< ValuePtr > fields;

  /** Is this tuple frozen, i.e., made immutable yet? */
  bool frozen;

  /** A sorted map of tag names to optional tag values.  Only
      initialized if there is at least one tag.  Only a single tag of
      each type is allowed; for multi-value tags, store a vector as the
      value. */
  std::map< string, ValuePtr > * _tags;

  /** The static counter of tuple IDs, unique to a process */
  static uint32_t
  _tupleIDCounter;


  /** My ID */
  uint32_t _ID;

public:
  
  Tuple();
  
  
  ~Tuple();
  
  
  /** Create a pointer to a new, empty, unfrozen tuple. This is the
      preferred way to create new tuples, as opposed to using the
      constructor.  */
  static TuplePtr
  mk();

  /** Create a pointer to a new unfrozen tuple, initialized with identifer in
      position 0 and the tuple name in position 1. This is the
      preferred way to create new tuples, as opposed to using the
      constructor.  */
  static TuplePtr
  mk(string name, bool id=false);

  /** Makes an unfrozen copy of the tuple */
  TuplePtr 
  clone(string name="", bool id=false) const;

  /** Prepend a value pointer to the end of an unfrozen tuple, keeping
      the tuple unfrozen. */
  void
  prepend(ValuePtr tf);
  
  /** Append a value pointer to the end of an unfrozen tuple, keeping
      the tuple unfrozen. */
  void
  append(ValuePtr tf);


  /** Append all the fields of the given tuple to the end of this tuple
   */
  void
  concat(TuplePtr t);


  /** Attach a named tag to the tuple. The tuple must not be frozen. To
      store a tag with no value use Val_Null::mk() to return the
      (single, static, constant) NULL P2 value, which is different from
      plain old NULL. */
  void
  tag(string, ValuePtr);


  /** Lookup a name tag in the tuple.  If not found, null is returned.
      If found, a real ValuePtr is returned. */
  ValuePtr
  tag(string);


  /** The tuple becomes immutable. It will not allow further appends or
      concats to itself. */
  void
  freeze();


  /** Return the number of fields in the tuple, excluding tuple metadata
      such as the tuple ID, any tags, etc. */
  uint32_t
  size() const;


  ValuePtr
  operator[] (ptrdiff_t i);


  const ValuePtr
  operator[] (ptrdiff_t i) const;


  /** This method only exists for Boost.Python. Use the operator[] for all 
    * other purposes */
  ValuePtr
  at(ptrdiff_t i) { return fields[i]; };


  /** Set the field at given position to a value. The tuple must be
      unfrozen and have enough room already. */
  void
  set(uint32_t, ValuePtr);

  void marshal(boost::archive::text_oarchive *xdrs);

  static TuplePtr unmarshal(boost::archive::text_iarchive *xdrs);

  string toString() const;

  string toConfString() const;

  /** Compare tuples, one field at a time.  Only compare user-visible
      (i.e., enumerable via tuple[fieldNo]) fields. For instance, do not
      compare pointers, and do not compare tuple IDs.  For tuples of
      different field counts, compare their field counts instead.  */
  int compareTo(TuplePtr) const;

  /** The empty untagged tuple. */
  static TuplePtr EMPTY;

  /** The empty static initializer class */
  class EmptyInitializer {
  public:
    EmptyInitializer() {
      EMPTY->freeze();
    }
  };

  /** My ID */
  uint32_t
  ID();


  /** A comparator object for tuples */
  struct Comparator
  {
    bool
    operator()(const TuplePtr first,
               const TuplePtr second) const;
  };




private:
  /** And the empty initializer object */
  static EmptyInitializer _theEmptyInitializer;
};



/** A set of tuples */
typedef std::set< TuplePtr, Tuple::Comparator > TupleSet;

#endif /* __TUPLE_H_ */
