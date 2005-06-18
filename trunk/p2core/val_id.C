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
 */

#include "val_id.h"

#include "val_uint32.h"
#include "val_uint64.h"
#include "oper.h"

class OperID : public OperCompare<Val_ID> {
  virtual ValuePtr _lshift (const ValueRef& v1, const ValueRef& v2) const {
    uint32_t s = Val_UInt32::cast(v1);
    IDRef   id = Val_ID::cast(v2);
    return Val_ID::mk(id->shift(s));
  };

  virtual ValuePtr _plus (const ValueRef& v1, const ValueRef& v2) const {
    IDRef id1 = Val_ID::cast(v1);
    IDRef id2 = Val_ID::cast(v2);
    return Val_ID::mk(id1->add(id2));
  };

  virtual ValuePtr _minus (const ValueRef& v1, const ValueRef& v2) const {
    IDRef id1 = Val_ID::cast(v1);
    IDRef id2 = Val_ID::cast(v2);
    return Val_ID::mk(id2->distance(id1));
  };
};
const Oper* Val_ID::oper_ = New OperID();

//
// Marshalling and unmarshallng
//
void Val_ID::xdr_marshal_subtype( XDR *x )
{
  i->xdr_marshal(x);
}

ValueRef Val_ID::xdr_unmarshal( XDR *x )
{
  return Val_ID::mk(ID::xdr_unmarshal(x));
}


//
// Casting
//
IDRef Val_ID::cast(ValueRef v) {
  Value *vp = v;
  switch (v->typeCode()) {
  case Value::ID:
    return (static_cast<Val_ID *>(vp))->i;
  case Value::UINT32:
    return ID::mk(Val_UInt32::cast(v));
  case Value::INT64:
    return ID::mk(Val_UInt64::cast(v));
  case Value::UINT64:
    return ID::mk(Val_UInt64::cast(v));
  default:
    throw Value::TypeError(v->typeCode(), Value::ID );
  }
}

int Val_ID::compareTo(ValueRef other) const
{
  if (other->typeCode() != Value::ID) {
    if (Value::ID < other->typeCode()) {
      return -1;
    } else if (Value::ID > other->typeCode()) {
      return 1;
    }
  }
  return i->compareTo(cast(other));
}

/*
 * End of file
 */
