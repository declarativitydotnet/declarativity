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


//
// Marshalling and unmarshallng
//
void Val_Int32::xdr_marshal( XDR *x )
{
  xdr_int32_t(x, &i);
}

//
// Casting
//
int32_t Val_Int32::cast(ValueRef v) {
  Value *vp = v;
  switch (v->typeCode()) {
  case Value::INT32:
    return (static_cast<Val_Int32 *>(vp))->i;
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
    return strtol(Val_Str::cast(v).cstr(), NULL, 0);
  default:
    throw Value::TypeError(v->typeCode(), Value::INT32 );
  }
}

/*
 * End of file
 */
