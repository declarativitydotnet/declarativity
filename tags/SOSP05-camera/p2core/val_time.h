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
 * DESCRIPTION: P2's concrete type system: time type.
 *
 */

#ifndef __VAL_TIME_H__
#define __VAL_TIME_H__

#include "value.h"
#include "oper.h"


class Val_Time : public Value {

public:  
  // Required fields for all concrete types.

  const Value::TypeCode typeCode() const { return Value::TIME; };

  // The type name
  const char *typeName() const { return "time"; };

  virtual str toString() const { return strbuf() << "[" << t.tv_sec << "," << t.tv_nsec << "]"; };
  virtual unsigned int size() const { return sizeof(struct timespec); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );

  static ValueRef xdr_unmarshal( XDR *x );

  // Constructors
  Val_Time(struct timespec theTime) : t(theTime) {};

  // Factory
  static ValueRef mk(struct timespec theTime) { return New refcounted<Val_Time>(theTime); };

  // Strict comparison
  int compareTo(ValueRef) const;

  // Casting
  static struct timespec cast(ValueRef v);

  const static opr::Oper* oper_;
private:
  /** The time */
  struct timespec t;

  /** A double to be used with modf */
  static double _theDouble;
};

#endif /* __VAL_TIME_H_ */
