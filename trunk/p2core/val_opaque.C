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
 * DESCRIPTION: P2's concrete type system: String type.
 *
 */

#include "val_opaque.h"

#include "val_str.h"
#include <arpc.h>

//
// Marshal a opaqueing
// 
void Val_Opaque::xdr_marshal_subtype( XDR *x ) 
{
  warn << "Cannot marshal an OPAQUE value\n";
}

//
// Casting
//
ref<suio> Val_Opaque::cast(ValueRef v)
{
  Value *vp = v;
  switch (v->typeCode()) {
  case Value::OPAQUE:
    return (static_cast<Val_Opaque *>(vp))->u;
  case Value::STR:
    {
      ref<suio> u = New refcounted<suio>();
      str s = Val_Str::cast(v);
      u->copy(s.cstr(),s.len());
      return u;
    }
  default:
    throw Value::TypeError(v->typeCode(), Value::OPAQUE );
  }
}
  
/* 
 * End of file
 */
