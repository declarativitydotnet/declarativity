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
 * DESCRIPTION: P2's concrete type system: UInt64 type.
 *
 */

#include "val_uint64.h"

#include "val_uint32.h"
#include "val_int32.h"
#include "val_int64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_null.h"

const opr::Oper* Val_UInt64::oper_ = new opr::OperImpl<Val_UInt64>();

string Val_UInt64::toConfString() const
{
  ostringstream conf;
  conf << "Val_UInt64(" << i << ")";
  return conf.str();
}

//
// Marshalling and unmarshallng
//
void Val_UInt64::xdr_marshal_subtype( XDR *x )
{
  xdr_uint64_t(x, &i);
}
ValuePtr Val_UInt64::xdr_unmarshal( XDR *x )
{
  uint64_t i;
  xdr_uint64_t(x, &i);
  return mk(i);
}

//
// Casting
//
uint64_t
Val_UInt64::cast(ValuePtr v) {
  uint64_t returnValue = (uint64_t) ((double) -1.0);
  switch (v->typeCode()) {
  case Value::UINT64:
    returnValue = (static_cast<Val_UInt64 *>(v.get()))->i;
    break;
  case Value::INT32:
    returnValue = (uint64_t)(Val_Int32::cast(v));
    break;
  case Value::UINT32:
    returnValue = (uint64_t)(Val_UInt32::cast(v));
    break;
  case Value::INT64:
    returnValue = (uint64_t)(Val_Int64::cast(v));
    break;
  case Value::DOUBLE:
    returnValue = (uint64_t) (Val_Double::cast(v));
    break;
  case Value::NULLV:
    returnValue = 0;
    break;
  case Value::STR:
    returnValue = strtoull(Val_Str::cast(v).c_str(),(char **)NULL, 0);
    break;
  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::UINT64,
                           "uint64");
  }
  return returnValue;
}

int Val_UInt64::compareTo(ValuePtr other) const
{
  if (other->typeCode() != Value::UINT64) {
    if (Value::UINT64 < other->typeCode()) {
      return -1;
    } else if (Value::UINT64 > other->typeCode()) {
      return 1;
    }
  }
  if (i < cast(other)) {
    return -1;
  } else if (i > cast(other)) {
    return 1;
  } else {
    return 0;
  }
}

/*
 * End of file
 */
