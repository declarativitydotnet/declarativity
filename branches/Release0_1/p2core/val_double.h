/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: P2's concrete type system: Double (floating point).
 *
 */

#ifndef __VAL_DOUBLE_H__
#define __VAL_DOUBLE_H__

#include "value.h"
#include "oper.h"


class Val_Double : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::DOUBLE; };
  const char *typeName() const { return "double"; };
  virtual string toString() const;
  virtual unsigned int size() const { return sizeof(double); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructor
  Val_Double(double theFloat) : d(theFloat) {};

  // Factory
  static ValuePtr mk(double d) { ValuePtr p(new Val_Double(d)); return p; };

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting
  static double cast(ValuePtr v);
  
  static const opr::Oper* oper_;
private:
  double d;
  
};

#endif /* __VAL_DOUBLE_H_ */
