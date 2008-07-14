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
 * DESCRIPTION: P2's concrete type system.  The base type for values.
 *
 */

#ifndef __VALUE_H__
#define __VALUE_H__

#include <vector>

#include <assert.h>
#include <async.h>
#include <arpc.h>
#include "inlines.h"

class Value;
typedef ref<Value> ValueRef;
typedef ptr<Value> ValuePtr;

class Value {

protected: 

  Value() {};

public:  

  // Type codes
  enum TypeCode { 
    NULLV=0,
    STR,
    INT32,
    UINT32,
    INT64,
    UINT64,
    DOUBLE,
    OPAQUE,
    TUPLE,
    TIME,
    ID
  };

  virtual unsigned int size() const = 0;
  
  // The type name
  virtual const TypeCode typeCode() const =0;
  virtual const char *typeName() const =0;

  // Conversions to strings: mandatory. 
  virtual str toString() const =0;
  str toTypeString() { return strbuf() << typeName() << ":" << toString();};

  /** Strict equality */
  REMOVABLE_INLINE bool equals( ValueRef other ) const { return compareTo(other) == 0; }

  /** Am I less than, equal or greater than the other value?  -1 means
      less, 0 means equal, +1 means greater. */
  virtual int compareTo( ValueRef other ) const = 0;

  // Thrown when an invalid type conversion is attempted. 
  struct TypeError { 
    TypeCode	realType;
    TypeCode	toType;
    TypeError(TypeCode t1, TypeCode t2) : realType(t1), toType(t2) {};
  };

  // Marshalling
  void xdr_marshal( XDR *x );
  static ValueRef xdr_unmarshal( XDR *x );
protected:
  virtual void xdr_marshal_subtype( XDR *x )=0;
};

#endif /* __VALUE_H_ */
