/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: P2's concrete type system: the tuple type (for
 * encapsulations).
 *
 */

#ifndef __VAL_TUPLE_H__
#define __VAL_TUPLE_H__

#include "value.h"
#include "tuple.h"
#include "oper.h"


class Val_Tuple : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::TUPLE; };
  const char *typeName() const { return "tuple"; };
  string toString() const { return t->toString(); };
  virtual unsigned int size() const { return (t ? t->size() : 0); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructor
  Val_Tuple(TuplePtr tuple) : t(tuple) {};
  virtual ~Val_Tuple() {};

  // Factory
  static ValuePtr mk(TuplePtr t) { ValuePtr p(new Val_Tuple(t)); return p; };

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting
  static TuplePtr cast(ValuePtr v);
  
  static const opr::Oper* oper_;
private:
  TuplePtr t;
};

#endif /* __VAL_TUPLE_H_ */
