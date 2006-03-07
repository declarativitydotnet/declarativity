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

using namespace opr;

class OperTime : public opr::OperCompare<Val_Time> {
  virtual ValuePtr _plus (const ValuePtr& v1, const ValuePtr& v2) const {
    struct timespec t1 = Val_Time::cast(v1);
    struct timespec t2 = Val_Time::cast(v2);
    return Val_Time::mk(t1 + t2);
  };

  virtual ValuePtr _minus (const ValuePtr& v1, const ValuePtr& v2) const {
    struct timespec t1 = Val_Time::cast(v1);
    struct timespec t2 = Val_Time::cast(v2);
    return Val_Time::mk(t1 - t2);
  };
};
const opr::Oper* Val_Time::oper_ = new OperTime();

//
// Marshalling and unmarshallng
//
void Val_Time::xdr_marshal_subtype( XDR *x )
{
  xdr_long(x, &(t.tv_sec));
  xdr_long(x, &(t.tv_nsec));
}

ValuePtr Val_Time::xdr_unmarshal( XDR *x )
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
struct timespec Val_Time::cast(ValuePtr v) {
  switch (v->typeCode()) {
  case Value::TIME:
    return (static_cast<Val_Time *>(v.get()))->t;
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
      TuplePtr theTuple = Val_Tuple::cast(v);
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

int Val_Time::compareTo(ValuePtr other) const
{
  if (other->typeCode() != Value::TIME) {
    if (Value::TIME < other->typeCode()) {
      return -1;
    } else if (Value::TIME > other->typeCode()) {
      return 1;
    }
  }
  if (t < cast(other)) {
    return -1;
  } else if (t > cast(other)) {
    return 1;
  } else {
    return 0;
  }
}

/*
 * End of file
 */
