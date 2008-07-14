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


#include "val_null.h"
#include "oper.h"


class OperNull : public opr::Oper {
public:
  bool
  _eq(const ValuePtr& v1, const ValuePtr& v2) const {
    return (v1->typeCode() == Value::NULLV) &&
      (v2->typeCode() == Value::NULLV);
  }; 


  bool
  _neq(const ValuePtr& v1, const ValuePtr& v2) const {
    return !OperNull::_eq(v1, v2);
  };
};

const opr::Oper* Val_Null::oper_ = new OperNull();

//
// Singleton null value.
//
ValuePtr Val_Null::singleton = ValuePtr(new Val_Null());

string Val_Null::toConfString() const
{
  return "Val_Null()";
}

//
// Marshalling and unmarshallng
//
void Val_Null::xdr_marshal_subtype( XDR *x ) 
{
  return;
}
ValuePtr Val_Null::xdr_unmarshal( XDR *x )
{
  return singleton;
}

int Val_Null::compareTo(ValuePtr other) const
{
  return (other->typeCode() != Value::NULLV);
}

//
// Casting: more for completeness than anything else...
// 
void Val_Null::cast(ValuePtr v) { 
  if (v->typeCode() != Value::NULLV) {
    throw Value::TypeError(v->typeCode(), Value::NULLV);
  };
}


