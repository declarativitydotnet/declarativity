/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION:
 *
 */

#ifndef __TUPLE_H__
#define __TUPLE_H__

#include <vector>
#include <map>
#include <utility>

#include <assert.h>
#include <async.h>
#include <arpc.h>
#include "inlines.h"
#include "value.h"

class Tuple;
typedef ref<Tuple> TupleRef;
typedef ptr<Tuple> TuplePtr;

class Tuple {

private:
  std::vector< ValueRef > fields;
  bool		frozen;

  /** A sorted map of tag names to optional tag values.  Only
      initialized if there is at least one tag.  Only a single tag of
      each type is allowed; for multi-value tags, store a vector as the
      value. */
  std::map< str, ValueRef > * _tags;

public:

  Tuple() : fields(), frozen(false), _tags(0) {};
  ~Tuple();

  static TupleRef mk() { return New refcounted<Tuple>(); };

  void append(ValueRef tf) { assert(!frozen); fields.push_back(tf); };

  /** Append all the fields of the given tuple to the end of this tuple
  */
  void concat(TupleRef t);

  /** Attach a named tag to the tuple. The tuple must not be frozen. To
      store a tag with no value use Val_Null::mk() to return the
      (single, static, constant) NULL P2 value, which is different from
      plain old NULL. */
  void tag(str, ValueRef);

  /** Lookup a name tag in the tuple.  If not found, null is returned.
      If found, a real ValueRef is returned. */
  ValuePtr tag(str);

  void freeze() { frozen = true; };

  size_t size() const { return fields.size(); };

  ValueRef operator[] (ptrdiff_t i) { return fields[i]; };
  const ValueRef operator[] (ptrdiff_t i) const { return fields[i]; };

  void xdr_marshal( XDR *uio );
  static TupleRef xdr_unmarshal( XDR *uio );

  str toString() const;

  /** Strict comparison, one field at a time. */
  int compareTo(TupleRef) const;

  /** The empty untagged tuple. */
  static TupleRef EMPTY;

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
