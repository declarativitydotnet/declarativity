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
#include <utility>

#include <assert.h>
#include "inlines.h"
#include "value.h"

class Tuple;
typedef boost::shared_ptr<Tuple> TuplePtr;

class Tuple {

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
  static uint
  _tupleIDCounter;

  /** My ID */
  uint _ID;



public:

  Tuple() : fields(), frozen(false), _tags(0), _ID(_tupleIDCounter++) {};
  ~Tuple();

  static TuplePtr mk() { TuplePtr p(new Tuple()); return p; };

  void append(ValuePtr tf) { assert(!frozen); fields.push_back(tf); };

  /** Append all the fields of the given tuple to the end of this tuple
  */
  void concat(TuplePtr t);

  /** Attach a named tag to the tuple. The tuple must not be frozen. To
      store a tag with no value use Val_Null::mk() to return the
      (single, static, constant) NULL P2 value, which is different from
      plain old NULL. */
  void tag(string, ValuePtr);

  /** Lookup a name tag in the tuple.  If not found, null is returned.
      If found, a real ValuePtr is returned. */
  ValuePtr tag(string);

  void freeze() { frozen = true; };

  size_t size() const { return fields.size(); };

  ValuePtr operator[] (ptrdiff_t i) { return fields[i]; };
  const ValuePtr operator[] (ptrdiff_t i) const { return fields[i]; };

  ValuePtr at(ptrdiff_t i) { return fields[i]; };

  void xdr_marshal( XDR *xdrs );
  static TuplePtr xdr_unmarshal( XDR *xdrs );

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

private:
  /** And the empty initializer object */
  static EmptyInitializer _theEmptyInitializer;
};

#endif /* __TUPLE_H_ */
