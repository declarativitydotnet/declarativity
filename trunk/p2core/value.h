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
    OPAQUE
  };
  
  // The type name
  virtual const TypeCode typeCode() const =0;
  virtual const char *typeName() const =0;

  virtual str toString() const =0;
  str toTypeString() { return strbuf() << typeName() << ":" << toString();};
  
  struct TypeError { 
    TypeCode	realType;
    TypeCode	toType;
    TypeError(TypeCode t1, TypeCode t2) : realType(t1), toType(t2) {};
  };

  // Marshalling and unmarshallng
  virtual void xdr_marshal( XDR *x ) =0;
  
};

#endif /* __VALUE_H_ */
