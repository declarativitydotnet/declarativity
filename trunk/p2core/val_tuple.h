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
  str toString() const { return t->toString(); };
  virtual unsigned int size() const { return (t ? t->size() : 0); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValueRef xdr_unmarshal( XDR *x );

  // Constructor
  Val_Tuple(TupleRef tuple) : t(tuple) {};
  virtual ~Val_Tuple() {};

  // Factory
  static ValueRef mk(TupleRef t) { return New refcounted<Val_Tuple>(t); };

  // Strict comparison
  int compareTo(ValueRef) const;

  // Casting
  static TupleRef cast(ValueRef v);
  
  const static opr::Oper* oper_;
private:
  TupleRef t;
};

#endif /* __VAL_TUPLE_H_ */
