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
 * DESCRIPTION: P2's concrete type system: UInt64 type.
 *
 */

#ifndef __VAL_UINT64_H__
#define __VAL_UINT64_H__

#include "value.h"

class Val_UInt64 : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::UINT64; };
  const char *typeName() const { return "uint64"; };
  virtual str toString() const { return strbuf() << i; };

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValueRef xdr_unmarshal( XDR *x );

  // Constructor
  Val_UInt64(uint64_t theInt) : i(theInt) {};

  // Factory
  static ValueRef mk(uint64_t i) { return New refcounted<Val_UInt64>(i); };

  // Strict comparison
  int compareTo(ValueRef) const;

  // Casting
  static uint64_t cast(ValueRef v);
  
private:
  uint64_t i;
  
};

#endif /* __VAL_UINT64_H_ */
