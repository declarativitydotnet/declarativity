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
 * DESCRIPTION: P2's concrete type system: Int64 type.
 *
 */

#include "val_int64.h"

#include "val_uint32.h"
#include "val_int32.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_null.h"


//
// Marshalling and unmarshallng
//
void Val_Int64::xdr_marshal_subtype( XDR *x )
{
  xdr_int64_t(x, &i);
}
ValueRef Val_Int64::xdr_unmarshal( XDR *x )
{
  int64_t i;
  xdr_int64_t(x, &i);
  return mk(i);
}

//
// Casting
//
int64_t Val_Int64::cast(ValueRef v) {
  Value *vp = v;
  switch (v->typeCode()) {
  case Value::INT64:
    return (static_cast<Val_Int64 *>(vp))->i;
  case Value::INT32:
    return (int64_t)(Val_Int32::cast(v));
  case Value::UINT32:
    return (int64_t)(Val_UInt32::cast(v));
  case Value::UINT64:
    return (int64_t)(Val_UInt64::cast(v));
  case Value::DOUBLE:
    return (int64_t)(Val_Double::cast(v));
  case Value::NULLV:
    return 0;
  case Value::STR:
    return strtoll(Val_Str::cast(v).cstr(),NULL,0);
  default:
    throw Value::TypeError(v->typeCode(), Value::INT64 );
  }
}

bool Val_Int64::equals(ValueRef other) const
{
  if (other->typeCode() != Value::INT64) {
    return false;
  }
  return cast(other) == i;
}

/*
 * End of file
 */
