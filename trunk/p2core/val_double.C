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
 * DESCRIPTION: P2's concrete type system: Double (floating point).
 *
 */

#include "val_double.h"

#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_str.h"
#include "val_null.h"

#define DOUBLE_HACK
#include "oper.h"
const Oper* Val_Double::oper_ = New OperImpl<Val_Double, double>();

//
// String conversion. 
// Yuck.  IF we convert libasync to the STL this will be easier.
// Note: toString converts to an _exact_ representation of the double
// (i.e. in binary), rather than the more human-familiar decimal
// version, which is what you get if you cast it to a string. 
//
str Val_Double::toString() const
{
  char dbuf[100];
  sprintf(dbuf,"%g",d);
  return strbuf() << dbuf;
}

//
// Marshalling and unmarshallng
//
void Val_Double::xdr_marshal_subtype( XDR *x )
{
  xdr_double(x, &d);
}
ValueRef Val_Double::xdr_unmarshal( XDR *x )
{
  double d;
  xdr_double(x, &d);
  return mk(d);
}

//
// Casting
//
double Val_Double::cast(ValueRef v)
{
  Value *vp = v;
  switch (v->typeCode()) {
  case Value::DOUBLE:
    return (static_cast<Val_Double *>(vp))->d;
  case Value::INT32:
    return (double)(Val_Int32::cast(v));
  case Value::UINT32:
    return (double)(Val_UInt32::cast(v));
  case Value::INT64:
    return (double)(Val_Int64::cast(v));
  case Value::UINT64:
    return (double)(Val_UInt64::cast(v));
  case Value::NULLV:
    return 0;
  case Value::STR:
    return strtod(Val_Str::cast(v).cstr(),NULL);
  default:
    throw Value::TypeError(v->typeCode(), Value::DOUBLE );
  }
}

int Val_Double::compareTo(ValueRef other) const
{
  if (other->typeCode() != Value::DOUBLE) {
    if (Value::DOUBLE < other->typeCode()) {
      return -1;
    } else if (Value::DOUBLE > other->typeCode()) {
      return 1;
    }
  }
  if (d < cast(other)) {
    return -1;
  } else if (d > cast(other)) {
    return 1;
  } else {
    return 0;
  }
}

/*
 * End of file
 */
