/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: P2's concrete type system: the NULL type.
 *
 */

#ifndef __VAL_NULL_H__
#define __VAL_NULL_H__

#include "value.h"
#include "oper.h"

class Val_Null : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::NULLV; };
  const char *typeName() const { return "null"; };
  str toString() const { return "NULL"; };
  virtual unsigned int size() const { return sizeof(singleton); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Factory
  static ValuePtr mk() { return singleton; };

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting: more for completeness than anything else...
  static void cast(ValuePtr v);
  
  static const opr::Oper* oper_;
private:
  static ValuePtr singleton;
};

#endif /* __VAL_NULL_H_ */
