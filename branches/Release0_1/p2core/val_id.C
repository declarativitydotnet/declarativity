/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "val_id.h"
#include "val_uint32.h"
#include "val_uint64.h"

class OperID : public opr::OperCompare<Val_ID> {
  virtual ValuePtr _lshift (const ValuePtr& v1, const ValuePtr& v2) const {
    IDPtr   id = Val_ID::cast(v1);
    uint32_t s = Val_UInt32::cast(v2);
    return Val_ID::mk(id->shift(s));
  };

  virtual ValuePtr _plus (const ValuePtr& v1, const ValuePtr& v2) const {
    IDPtr id1 = Val_ID::cast(v1);
    IDPtr id2 = Val_ID::cast(v2);
    return Val_ID::mk(id1->add(id2));
  };

  virtual ValuePtr _minus (const ValuePtr& v1, const ValuePtr& v2) const {
    IDPtr id1 = Val_ID::cast(v1);
    IDPtr id2 = Val_ID::cast(v2);
    return Val_ID::mk(id2->distance(id1));
  };

  virtual ValuePtr _dec (const ValuePtr& v1) const {
    IDPtr id1 = Val_ID::cast(v1);
    return Val_ID::mk(ID::ONE->distance(id1));
  };

  virtual ValuePtr _inc (const ValuePtr& v1) const {
    IDPtr id1 = Val_ID::cast(v1);
    return Val_ID::mk(id1->add(ID::ONE));
  };

};
const opr::Oper* Val_ID::oper_ = new OperID();

//
// Marshalling and unmarshallng
//
void Val_ID::xdr_marshal_subtype( XDR *x )
{
  i->xdr_marshal(x);
}

ValuePtr Val_ID::xdr_unmarshal( XDR *x )
{
  return Val_ID::mk(ID::xdr_unmarshal(x));
}


//
// Casting
//  no negative values allowed
//
IDPtr Val_ID::cast(ValuePtr v) {
  switch (v->typeCode()) {
  case Value::ID:
    return (static_cast<Val_ID *>(v.get()))->i;
  case Value::INT32: {
    if (Val_UInt32::cast(v) < 0)
      throw Value::TypeError(v->typeCode(), Value::ID );
    return ID::mk(Val_UInt32::cast(v));
  }
  case Value::INT64: {
    if (Val_UInt64::cast(v) < 0)
      throw Value::TypeError(v->typeCode(), Value::ID );
    return ID::mk(Val_UInt64::cast(v));
  }
  case Value::UINT32:
    return ID::mk(Val_UInt32::cast(v));
  case Value::UINT64:
    return ID::mk(Val_UInt64::cast(v));
  default:
    throw Value::TypeError(v->typeCode(), Value::ID );
  }
}

int Val_ID::compareTo(ValuePtr other) const
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
