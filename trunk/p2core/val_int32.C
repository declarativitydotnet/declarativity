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
 * DESCRIPTION: P2's concrete type system: Int32 type.
 *
 */

#include "val_int32.h"

#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_null.h"

const opr::Oper* Val_Int32::oper_ = new opr::OperImpl<Val_Int32>();

//
// Marshalling and unmarshallng
//
void Val_Int32::xdr_marshal_subtype( XDR *x )
{
  xdr_int32_t(x, &i);
}
ValuePtr Val_Int32::xdr_unmarshal( XDR *x )
{
  int32_t i;
  xdr_int32_t(x, &i);
  return mk(i);
}

string Val_Int32::toConfString() const
{
  ostringstream conf;
  conf << "Val_Int32(" << i << ")";
  return conf.str();
}

//
// Casting
//
int32_t Val_Int32::cast(ValuePtr v) {
  ValuePtr vp = v;
  switch (v->typeCode()) {
  case Value::INT32:
    return (static_cast<Val_Int32*>(vp.get()))->i;
  case Value::UINT32:
    return (int32_t)(Val_UInt32::cast(v));
  case Value::INT64:
    return (int32_t)(Val_Int64::cast(v));
  case Value::UINT64:
    return (int32_t)(Val_UInt64::cast(v));
  case Value::DOUBLE:
    return (int32_t)(Val_Double::cast(v));
  case Value::NULLV:
    return 0;
  case Value::STR:
    return strtol(Val_Str::cast(v).c_str(), NULL, 0);
  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::INT32,
                           "int32");
  }
}

int Val_Int32::compareTo(ValuePtr other) const
{
  if (Value::INT32 < other->typeCode()) {
    return -1;
  } else if (Value::INT32 > other->typeCode()) {
    return 1;
  } else if (i < cast(other)) {
    return -1;
  } else if (i > cast(other)) {
    return 1;
  } else {
    return 0;
  }
}

/** Define the ZERO constant */
ValuePtr Val_Int32::ZERO = Val_Int32::mk(0);


/*
 * End of file
 */
