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
 * DESCRIPTION: P2's concrete type system: String type.
 *
 */

#ifndef __VAL_STR_H__
#define __VAL_STR_H__

#include "value.h"
#include "oper.h"

class Val_Str : public Value {

public:  
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::STR; };
  const char *typeName() const { return "str"; };
  string toString() const { return s; };
  virtual string toConfString() const;
  virtual unsigned int size() const { return s.size(); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructor
  Val_Str(string theString) : s(theString) {};
  virtual ~Val_Str() {};

  // Factory
  static ValuePtr mk(string s) { ValuePtr p(new Val_Str(s)); return p; };

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting
  static string cast(ValuePtr v);
  static string raw_val(Val_Str& v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }
  
  static const opr::Oper *oper_;
private:
  string s;
};

#endif /* __VAL_STR_H_ */
