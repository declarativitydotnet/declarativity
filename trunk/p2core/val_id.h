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
 * DESCRIPTION: P2's concrete type system: ring identifier.
 *
 */

#ifndef __VAL_ID_H__
#define __VAL_ID_H__

#include "value.h"
#include "ID.h"

class Oper;

class Val_ID : public Value {

public:  
  // Required fields for all concrete types.

  const Value::TypeCode typeCode() const { return Value::ID; };

  // The type name
  const char *typeName() const { return "ID"; };

  virtual str toString() const { return strbuf() << i->toString(); };
  virtual unsigned int size() const { return sizeof(ID); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );

  static ValueRef xdr_unmarshal( XDR *x );

  // Constructors
  Val_ID(IDRef theID) : i(theID) {};
  virtual ~Val_ID() {};

  // Factory
  static ValueRef mk(IDRef theID) { return New refcounted<Val_ID>(theID); };

  // Strict comparison
  int compareTo(ValueRef) const;

  // Casting
  static IDRef cast(ValueRef v);

  const static Oper* oper_;
private:
  /** The ID */
  IDRef i;
};

#endif /* __VAL_ID_H_ */
