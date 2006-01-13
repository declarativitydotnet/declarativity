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
 * DESCRIPTION: P2's concrete type system: String type.
 *
 */

#include <iostream>
#include "val_str.h"
#include "val_double.h"

class OperStr : public opr::OperCompare<Val_Str> {
  virtual ValuePtr _plus (const ValuePtr& v1, const ValuePtr& v2) const {
    return Val_Str::mk( Val_Str::cast(v1) + Val_Str::cast(v2) );
  };
};
const opr::Oper* Val_Str::oper_ = new OperStr();

//
// Marshal a string
// 
void Val_Str::xdr_marshal_subtype( XDR *x )
{
  const char *st = s.c_str();
  long sl = s.length();
  xdr_long(x, &sl);
  xdr_string(x, const_cast<char **>(&st), sl + 1);
}

/** The preallocated string buffer used for unmarshaling strings */
const int STATIC_STRING_BUFFER = 10000;

ValuePtr Val_Str::xdr_unmarshal( XDR *x )
{
  long sl;
  xdr_long(x, &sl);
  // Now fetch the string itself
  static char stringBuffer[STATIC_STRING_BUFFER];
  
  if (sl + 1 <= STATIC_STRING_BUFFER) {
    // We can use the static buffer
    xdr_string(x, (char**) &stringBuffer, sl + 1);
    stringBuffer[sl] = 0;       // make sure it's null terminated
    string st(stringBuffer, sl);
    return mk(st);
  }  else {
    // We can't use the static buffer. We must allocate a one-shot
    // buffer
    char * localBuffer = new char[sl + 1];
    xdr_string(x, &localBuffer, sl);
    localBuffer[sl] = 0;
    string st(localBuffer, sl);
    delete localBuffer;
    return mk(st);
  }
}

int Val_Str::compareTo(ValuePtr other) const
{
  if (other->typeCode() != Value::STR) {
    return false;
  }
  return s.compare(cast(other));
}

//
// Casting: we special-case doubles...
//
string Val_Str::cast(ValuePtr v)
{
  return v->toString();
}

/* 
 * End of file
 */
