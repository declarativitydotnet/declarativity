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
 * DESCRIPTION: P2's concrete type system: Double (floating point).
 *
 */

#include "val_double.h"

#include "val_int64.h"
#include "val_str.h"
#include "val_null.h"

class OperDouble : public opr::OperCompare<Val_Double> {
  virtual ValuePtr _plus (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Double::mk(Val_Double::cast(v1) + Val_Double::cast(v2));
  };
  virtual ValuePtr _minus (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Double::mk(Val_Double::cast(v1) - Val_Double::cast(v2));
  };
  virtual ValuePtr _neg (const ValuePtr& v1) const {
    return Val_Double::mk(-Val_Double::cast(v1));
  };
  virtual ValuePtr _times (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Double::mk(Val_Double::cast(v1) * Val_Double::cast(v2));
  };
  virtual ValuePtr _divide (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Double::mk(Val_Double::cast(v1) / Val_Double::cast(v2));
  };
  virtual ValuePtr _dec (const ValuePtr& v1) const {
    return Val_Double::mk((Val_Double::cast(v1)) - 1.);
  };
  virtual ValuePtr _inc (const ValuePtr& v1) const {
    return Val_Double::mk((Val_Double::cast(v1)) + 1.);
  };
};
const opr::Oper* Val_Double::oper_ = new OperDouble();

//
// String conversion. 
// Yuck.  IF we convert libasync to the STL this will be easier.
// Note: toString converts to an _exact_ representation of the double
// (i.e. in binary), rather than the more human-familiar decimal
// version, which is what you get if you cast it to a string. 
//
string Val_Double::toString() const
{
  ostringstream sb;
  sb << d;
  return sb.str();
}

string Val_Double::toConfString() const
{
  return toString();
}

//
// Marshalling and unmarshallng
//
void Val_Double::xdr_marshal_subtype( XDR *x )
{
  xdr_double(x, &d);
}
ValuePtr Val_Double::xdr_unmarshal( XDR *x )
{
  double d;
  xdr_double(x, &d);
  return mk(d);
}

//
// Casting
//
double
Val_Double::cast(ValuePtr v)
{
  double returnValue;
  switch (v->typeCode()) {
  case Value::DOUBLE:
    returnValue = (static_cast<Val_Double *>(v.get()))->d;
    break;
  case Value::INT64:
    returnValue = (double)(Val_Int64::cast(v));
    break;
  case Value::NULLV:
    returnValue = 0;
    break;
  case Value::STR:
    returnValue = strtod(Val_Str::cast(v).c_str(),NULL);
    break;
  default:
    throw Value::TypeError(v->typeCode(),
                           v->typeName(),
                           Value::DOUBLE,
                           "double");
  }
  return returnValue;
}

int Val_Double::compareTo(ValuePtr other) const
{
  if (Value::DOUBLE < other->typeCode()) {
    return -1;
  } else if (Value::DOUBLE > other->typeCode()) {
    return 1;
  } else if (d < cast(other)) {
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
