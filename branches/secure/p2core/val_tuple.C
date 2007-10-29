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
 */

#include "val_tuple.h"
#include<iostream>
class OperTuple : public opr::OperCompare<Val_Tuple> {
  virtual bool _eq(const ValuePtr& v1, const ValuePtr& v2) const {
    ValuePtr c1 = Val_Tuple::mk(Val_Tuple::cast(v1));
    //    std::cout<<"comparing == " << v1->toString() << " and " << v2->toString();
    return c1->compareTo(v2) == 0;
  };
  virtual bool _neq (const ValuePtr& v1, const ValuePtr& v2) const {
    ValuePtr c1 = Val_Tuple::mk(Val_Tuple::cast(v1));
    //    std::cout<<"!comparing == " << v1->toString() << " and " << v2->toString();
    return c1->compareTo(v2) != 0;
  };

};


const opr::Oper* Val_Tuple::oper_ = new OperTuple();

//
// Marshal a tuple
// 
void Val_Tuple::xdr_marshal_subtype( XDR *x ) 
{
  t->xdr_marshal(x);
}

ValuePtr Val_Tuple::xdr_unmarshal( XDR *x )
{
  TuplePtr t = Tuple::xdr_unmarshal(x);
  return mk(t);
}
  
int Val_Tuple::compareTo(ValuePtr other) const
{
   if(Value::TUPLE < other->typeCode()) {
      return -1;
   } else if(Value::TUPLE > other->typeCode()) {
      return 1;
   } else if (!t) {
      return other->typeCode() == Value::NULLV ? 0 : 1;
   } else {
      return t->compareTo(cast(other));
   }
}


string Val_Tuple::toConfString() const
{
  return t->toConfString();
}


//
// Casting
//
TuplePtr Val_Tuple::cast(ValuePtr v) {
  if (v->typeCode() == Value::TUPLE) {
    return (static_cast< Val_Tuple * >(v.get()))->t;
  } else if (v->typeCode() == Value::NULLV) {
    return TuplePtr();
  } else {
    throw Value::TypeError(v->typeCode(), v->typeName(),
                           Value::TUPLE, "tuple");
  }
}

/* 
 * End of file
 */
