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
 * DESCRIPTION: P2's concrete type system: Opaque byte vectors (for
 *              e.g. packets).
 *
 */

#ifndef __VAL_OPAQUE_H__
#define __VAL_OPAQUE_H__

#include "value.h"

class Val_Opaque : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::OPAQUE; };
  const char *typeName() const { return "opaque"; };
  str toString() const { return strbuf(u); };

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );

  // Constructor
  Val_Opaque(ref<suio> uio) : u(uio) {};
  virtual ~Val_Opaque() {};

  // Factory
  static ValueRef mk(ref<suio> uio) {
    return New refcounted<Val_Opaque>(uio); };

  // Casting
  static ref<suio> cast(ValueRef v);
  
private:
  ref<suio> u;
  
};

#endif /* __VAL_OPAQUE_H_ */
