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
 * DESCRIPTION: P2's concrete type system: Double (floating point).
 *
 */

#ifndef __VAL_DOUBLE_H__
#define __VAL_DOUBLE_H__

#include "value.h"

class Val_Double : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::DOUBLE; };
  const char *typeName() const { return "double"; };
  virtual str toString() const;

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValueRef xdr_unmarshal( XDR *x );

  // Constructor
  Val_Double(double theFloat) : d(theFloat) {};

  // Factory
  static ValueRef mk(double d) { return New refcounted<Val_Double>(d); };

  // Strict comparison
  bool equals(ValueRef) const;

  // Casting
  static double cast(ValueRef v);
  
private:
  double d;
  
};

#endif /* __VAL_DOUBLE_H_ */
