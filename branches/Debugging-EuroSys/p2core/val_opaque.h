/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: Opaque byte vectors (for
 *              e.g. packets).
 *
 */

#ifndef __VAL_OPAQUE_H__
#define __VAL_OPAQUE_H__

#include "value.h"
#include "oper.h"
#include "fdbuf.h"
#include "loop.h"

class Val_Opaque : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::OPAQUE; };
  const char *typeName() const { return "opaque"; };
  string toString() const { return b->str(); };
  virtual unsigned int size() const { return (b ? b->length() : 0); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );

  // Constructor
  Val_Opaque(FdbufPtr fb) : b(fb) {};
  virtual ~Val_Opaque() {};

  // Factory
  static ValuePtr mk(FdbufPtr fb) {
    return ValuePtr(new Val_Opaque(fb)); };

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting
  static FdbufPtr cast(ValuePtr v);
  
  static const opr::Oper* oper_;
private:
  FdbufPtr b;
  
};

#endif /* __VAL_OPAQUE_H_ */
