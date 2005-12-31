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
    str s1 = Val_Str::cast(v1);
    str s2 = Val_Str::cast(v2);
    return Val_Str::mk((strbuf() << s1 << s2));
  };
};
const opr::Oper* Val_Str::oper_ = New OperStr();

//
// Marshal a string
// 
void Val_Str::xdr_marshal_subtype( XDR *x ) 
{
  //const char *st = s.cstr();
  //  xdr_wrapstring(x,(char **)&st);
  rpc_str<RPC_INFINITY> rs(s);
  rpc_traverse(x,rs);
}
ValuePtr Val_Str::xdr_unmarshal( XDR *x )
{
  // Note that this looks like a yucky double copy, but at least the
  // string data itself isn't copied (since rpc_str <: str).
  //char *st;
  //xdr_wrapstring(x,&st);
  rpc_str<RPC_INFINITY> rs;
  rpc_traverse(x,rs);
  return mk(str(rs));
}
  
int Val_Str::compareTo(ValuePtr other) const
{
  if (other->typeCode() != Value::STR) {
    return false;
  }
  return s.cmp(cast(other));
}

//
// Casting: we special-case doubles...
//
str Val_Str::cast(ValuePtr v)
{
  if (v->typeCode() == Value::DOUBLE ) {
    char dbuf[100];
    sprintf(dbuf,"%a",Val_Double::cast(v));
    return strbuf() << dbuf;
  } else {
    return v->toString();
  }
}

/* 
 * End of file
 */
