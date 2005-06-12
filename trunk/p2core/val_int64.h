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
 * DESCRIPTION: P2's concrete type system: Int64 type.
 *
 */

#ifndef __VAL_INT64_H__
#define __VAL_INT64_H__

#include "value.h"

class Oper;

class Val_Int64 : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::INT64; };
  const char *typeName() const { return "int64"; };
  virtual str toString() const { return strbuf() << i; };
  virtual unsigned int size() const { return sizeof(int64_t); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValueRef xdr_unmarshal( XDR *x );

  // Constructor
  Val_Int64(int64_t theInt) : i(theInt) {};

  // Factory
  static ValueRef mk(int64_t i) { return New refcounted<Val_Int64>(i); };

  // Strict comparison
  int compareTo(ValueRef) const;

  // Casting
  static int64_t cast(ValueRef v);
  
  const static Oper* oper_;
private:
  int64_t i;
  
};

#endif /* __VAL_INT64_H_ */
