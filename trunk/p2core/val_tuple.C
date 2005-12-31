/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "val_tuple.h"

const opr::Oper* Val_Tuple::oper_ = New opr::OperCompare<Val_Tuple>();

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
  if (other->typeCode() != Value::TUPLE) {
    return false;
  }
  return t->compareTo(cast(other));
}

//
// Casting
//
TuplePtr Val_Tuple::cast(ValuePtr v) {
  if (v->typeCode() == Value::TUPLE) {
    return (static_cast< Val_Tuple * >(v.get()))->t;
  } else {
    throw Value::TypeError(v->typeCode(), Value::TUPLE);
  }
}

/* 
 * End of file
 */
