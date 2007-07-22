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
 * DESCRIPTION: P2's concrete type system: UInt32 type.
 *
 */

#include "val_uint32.h"

#include "val_int32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_str.h"
#include "val_null.h"
// the boost serialization implementer claims text is not much more expensive than portable binary
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


const opr::Oper* Val_UInt32::oper_ = new opr::OperImpl<Val_UInt32>();

string Val_UInt32::toConfString() const
{
  ostringstream conf;
  conf << "Val_UInt32(" << i << ")";
  return conf.str();
}

//
// Marshalling and unmarshallng
//
void Val_UInt32::marshal_subtype( boost::archive::text_oarchive *x )
{
  *x & i;
}
ValuePtr Val_UInt32::unmarshal( boost::archive::text_iarchive *x )
{
  uint32_t i;
  *x & i;
  return mk(i);
}

//
// Casting
//
uint32_t Val_UInt32::cast(ValuePtr v) {
  switch (v->typeCode()) {
  case Value::UINT32:
    return (static_cast<Val_UInt32 *>(v.get()))->i;
  case Value::INT32:
    return (uint32_t)(Val_Int32::cast(v));
  case Value::INT64:
    return (uint32_t)(Val_Int64::cast(v));
  case Value::UINT64:
    return (uint32_t)(Val_UInt64::cast(v));
  case Value::DOUBLE:
    return (uint32_t)(Val_Double::cast(v));
  case Value::NULLV:
    return 0;
  case Value::STR:
    return strtoul(Val_Str::cast(v).c_str(), (char **)NULL, 0);
  default:
    throw Value::TypeError(v->typeCode(), v->typeName(),
                           Value::UINT32,
                           "uint32");
  }
}

int Val_UInt32::compareTo(ValuePtr other) const
{
  if (other->typeCode() != Value::UINT32) {
    if (Value::UINT32 < other->typeCode()) {
      return -1;
    } else if (Value::UINT32 > other->typeCode()) {
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
