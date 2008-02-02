/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
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
  string toString() const;
  virtual string toConfString() const;
  virtual unsigned int
  size() const;

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Factory
  static ValuePtr
  mk();

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting: more for completeness than anything else...
  static void cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { cast(other); return mk(); }
  
  static const opr::Oper* oper_;
};

#endif /* __VAL_NULL_H_ */
