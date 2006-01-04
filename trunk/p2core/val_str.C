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
  xdr_string(x, const_cast<char **>(&st), s.length() + 1);
}
ValuePtr Val_Str::xdr_unmarshal( XDR *x )
{
  long sl;
  xdr_long(x, &sl);
  char *cp = reinterpret_cast<char *>(xdr_inline(x, sl));
  if (cp) {
    return mk(string(cp,sl));
  } else {
    // Yuck. 
    string st(sl,0);
    for( ssize_t i=0; i<sl; i++) {
      char c;
      xdr_char(x, &c);
      st[i] = c;
    }
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
