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

const opr::Oper* Val_Int64::oper_ = new opr::OperImpl<Val_Int64>();

//
// Marshalling and unmarshallng
//
void Val_Int64::marshal_subtype( boost::archive::text_oarchive *x )
{
  *x & i;
}
ValuePtr Val_Int64::unmarshal( boost::archive::text_iarchive *x )
{
  int64_t i;
  *x & i;
  return mk(i);
}

string Val_Int64::toConfString() const
{
  ostringstream conf;
  conf << "Val_Int64(" << i << ")";
  return conf.str();
}

//
// Casting
//
int64_t Val_Int64::cast(ValuePtr v) {
  switch (v->typeCode()) {
  case Value::INT64:
    return (static_cast<Val_Int64 *>(v.get()))->i;
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
    return /* strtoll */_strtoi64(Val_Str::cast(v).c_str(),NULL,0);
  default:
    throw Value::TypeError(v->typeCode(), v->typeName(),
                           Value::INT64, "int64");
  }
}

int Val_Int64::compareTo(ValuePtr other) const
{
  if (Value::INT64 < other->typeCode()) {
    return -1;
  } else if (Value::INT64 > other->typeCode()) {
    return 1;
  } else if (i < cast(other)) {
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
