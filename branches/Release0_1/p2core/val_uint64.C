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
uint64_t Val_UInt64::cast(ValuePtr v) {
  switch (v->typeCode()) {
  case Value::UINT64:
    return (static_cast<Val_UInt64 *>(v.get()))->i;
  case Value::INT32:
    return (uint64_t)(Val_Int32::cast(v));
  case Value::UINT32:
    return (uint64_t)(Val_UInt32::cast(v));
  case Value::INT64:
    return (uint64_t)(Val_Int64::cast(v));
  case Value::DOUBLE:
    return (uint64_t)(Val_Double::cast(v));
  case Value::NULLV:
    return 0;
  case Value::STR:
    return strtoull(Val_Str::cast(v).c_str(),(char **)NULL, 0);
  default:
    throw Value::TypeError(v->typeCode(), Value::UINT64 );
  }
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
