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
 * DESCRIPTION: P2's concrete type system: UInt32 type.
 *
 */

#ifndef __VAL_UINT32_H__
#define __VAL_UINT32_H__

#include "value.h"
#include "oper.h"

class Val_UInt32 : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::UINT32; };
  const char *typeName() const { return "uint32"; };
  virtual str toString() const { return strbuf() << i; };
  virtual unsigned int size() const { return sizeof(uint32_t); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValueRef xdr_unmarshal( XDR *x );

  // Constructor
  Val_UInt32(uint32_t theInt) : i(theInt) {};

  // Factory
  static ValueRef mk(uint32_t i) { return New refcounted<Val_UInt32>(i); };

  // Strict comparison
  int compareTo(ValueRef) const;

  // Casting
  static uint32_t cast(ValueRef v);
  
  static const opr::Oper* oper_;
private:
  uint32_t i;
  
};

#endif /* __VAL_UINT32_H_ */
