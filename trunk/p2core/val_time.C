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

#include "val_time.h"

#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_null.h"
#include "val_tuple.h"

#include "math.h"

//
// Marshalling and unmarshallng
//
void Val_Time::xdr_marshal_subtype( XDR *x )
{
  xdr_long(x, &(t.tv_sec));
  xdr_long(x, &(t.tv_nsec));
}

ValueRef Val_Time::xdr_unmarshal( XDR *x )
{
  struct timespec t;
  xdr_long(x, &(t.tv_sec));
  xdr_long(x, &(t.tv_nsec));
  return mk(t);
}

double Val_Time::_theDouble = 0;

//
// Casting
//
struct timespec Val_Time::cast(ValueRef v) {
  Value *vp = v;
  switch (v->typeCode()) {
  case Value::TIME:
    return (static_cast<Val_Time *>(vp))->t;
  case Value::INT32:
    {
      struct timespec t;
      t.tv_sec = Val_Int32::cast(v);
      t.tv_nsec = 0;
      return t;
    }
  case Value::UINT32:
    {
      struct timespec t;
      t.tv_sec = Val_UInt32::cast(v);
      t.tv_nsec = 0;
      return t;
    }
  case Value::INT64:
    {
      struct timespec t;
      t.tv_sec = Val_Int64::cast(v);
      t.tv_nsec = 0;
      return t;
    }
  case Value::UINT64:
    {
      struct timespec t;
      t.tv_sec = Val_UInt64::cast(v);
      t.tv_nsec = 0;
      return t;
    }
  case Value::DOUBLE:
    {
      struct timespec t;
      double d = Val_Double::cast(v);
      t.tv_sec = (long) trunc(d);
      t.tv_nsec = (long) modf(d, &Val_Time::_theDouble);
      return t;
    }
  case Value::NULLV:
    {
      struct timespec t;
      t.tv_sec = 0;
      t.tv_nsec = 0;
      return t;
    }
  case Value::TUPLE:
    {
      struct timespec t;
      TupleRef theTuple = Val_Tuple::cast(v);
      if (theTuple->size() >= 2) {
        t.tv_sec = Val_Int32::cast((*theTuple)[0]);
        t.tv_nsec = Val_Int32::cast((*theTuple)[1]);
      } else {
        t.tv_sec = 0;
        t.tv_nsec = 0;
      }
      return t;
    }
  default:
    throw Value::TypeError(v->typeCode(), Value::TIME );
  }
}

int Val_Time::compareTo(ValueRef other) const
{
  if (other->typeCode() != Value::TIME) {
    if (Value::TIME < other->typeCode()) {
      return -1;
    } else if (Value::TIME > other->typeCode()) {
      return 1;
    }
  }
  return tscmp(t, cast(other));
}

/*
 * End of file
 */
