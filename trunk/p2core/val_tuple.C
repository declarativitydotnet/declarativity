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

#include <arpc.h>

//
// Marshal a tuple
// 
void Val_Tuple::xdr_marshal_subtype( XDR *x ) 
{
  t->xdr_marshal(x);
}

ValueRef Val_Tuple::xdr_unmarshal( XDR *x )
{
  TupleRef t = Tuple::xdr_unmarshal(x);
  return mk(t);
}
  
int Val_Tuple::compareTo(ValueRef other) const
{
  if (other->typeCode() != Value::TUPLE) {
    return false;
  }
  return t->compareTo(cast(other));
}

//
// Casting
//
TupleRef Val_Tuple::cast(ValueRef v) {
  Value *vp = v;
  if (v->typeCode() == Value::TUPLE) {
    return (static_cast< Val_Tuple * >(vp))->t;
  } else {
    throw Value::TypeError(v->typeCode(), Value::TUPLE);
  }
}

/* 
 * End of file
 */
