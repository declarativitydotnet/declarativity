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

#include "val_str.h"
#include "val_double.h"

#include <arpc.h>

//
// Marshal a string
// 
void Val_Str::xdr_marshal_subtype( XDR *x ) 
{
  rpc_str<RPC_INFINITY> rs(s);
  rpc_traverse(x,rs);
}
ValueRef Val_Str::xdr_unmarshal( XDR *x )
{
  // Note that this looks like a yucky double copy, but at least the
  // string data itself isn't copied (since rpc_str <: str).
  rpc_str<RPC_INFINITY> rs;
  rpc_traverse(x,rs);
  return mk(rs);
}
  
bool Val_Str::equals(ValueRef other) const
{
  if (other->typeCode() != Value::STR) {
    return false;
  }
  return cast(other) == s;
}

//
// Casting: we special-case doubles...
//
str Val_Str::cast(ValueRef v)
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
