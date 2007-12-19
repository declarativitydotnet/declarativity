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

string Val_Null::toString() const
{
  return "NULL";
};

string Val_Null::toConfString() const
{
  return toString();
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
  return Val_Null::mk();
}

int Val_Null::compareTo(ValuePtr other) const
{
  if (other->typeCode() == Value::NULLV)
    return 0;
  return -1; // NULL IS LESS THAN ANYTHING ELSE
}

//
// Casting: more for completeness than anything else...
// 
void Val_Null::cast(ValuePtr v) { 
  if (v->typeCode() != Value::NULLV) {
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::NULLV,
                           "null");
  };
}


ValuePtr
Val_Null::mk()
{
  static ValuePtr _theSingleton = ValuePtr(new Val_Null());
  return _theSingleton;
}


unsigned int
Val_Null::size() const
{
  return sizeof(Val_Null::mk());
}
