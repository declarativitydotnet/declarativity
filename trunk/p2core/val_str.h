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
 * DESCRIPTION: P2's concrete type system: String type.
 *
 */

#ifndef __VAL_STR_H__
#define __VAL_STR_H__

#include "value.h"

class Val_Str : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::STR; };
  const char *typeName() const { return "str"; };
  str toString() const { return s; };

  // Marshalling and unmarshallng
  void xdr_marshal( XDR *x );

  // Constructor
  Val_Str(str theString) : s(theString) {};
  virtual ~Val_Str() {};

  // Factory
  static ValueRef mk(str s) { return New refcounted<Val_Str>(s); };

  // Casting
  static str cast(ValueRef v) { return v->toString(); };
  
private:
  str s;
  
};

#endif /* __VAL_STR_H_ */
