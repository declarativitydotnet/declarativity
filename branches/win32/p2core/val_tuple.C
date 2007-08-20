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

const opr::Oper* Val_Tuple::oper_ = new opr::OperCompare<Val_Tuple>();

//
// Marshal a tuple
// 
void Val_Tuple::marshal_subtype( boost::archive::text_oarchive *x ) 
{
  t->marshal(x);
}

ValuePtr Val_Tuple::unmarshal( boost::archive::text_iarchive *x )
{
  TuplePtr t = Tuple::unmarshal(x);
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
  ostringstream conf;
  conf << "<";
  for (unsigned i = 0; i < t->size(); i++) {
    conf << (*t)[i]->toConfString();
    if (i < t->size() - 1)
      conf << ", ";
  }
  conf << ">";
  return conf.str();
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
