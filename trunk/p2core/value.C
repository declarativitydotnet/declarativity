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
 * DESCRIPTION: P2's concrete type system.  The base type for values.
 *
 */

#include "value.h"
#include "trace.h"

#include "val_null.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"
#include "val_int64.h"
#include "val_uint64.h"
#include "val_double.h"
#include "val_opaque.h"
#include "val_tuple.h"


typedef ValueRef (*_unmarshal_fn)( XDR *);

static _unmarshal_fn jump_tab[] = {
  Val_Null::xdr_unmarshal,
  Val_Str::xdr_unmarshal,
  Val_Int32::xdr_unmarshal,
  Val_UInt32::xdr_unmarshal,
  Val_Int64::xdr_unmarshal,
  Val_UInt64::xdr_unmarshal,
  Val_Double::xdr_unmarshal,
  Val_Opaque::xdr_unmarshal,
  Val_Tuple::xdr_unmarshal
};

//
// Marshalling
//
void Value::xdr_marshal( XDR *x ) 
{
  TRC_FN;
  TypeCode tc = typeCode();
  TRC("TypeCode is " << tc);
  xdr_u_int32_t( x, &tc);
  xdr_marshal_subtype(x);
}

//
// Unmarshalling
//
ValueRef Value::xdr_unmarshal( XDR *x )
{
  TRC_FN;
  TypeCode tc;
  xdr_u_int32_t( x, &tc);
  TRC("TypeCode is " << tc);
  if ( (unsigned)tc >= (sizeof(jump_tab)/sizeof(_unmarshal_fn))) {
    warn << "Unmarshalling: Bad typecode " << tc << "\n";
    return Val_Null::mk();
  } else {
    return jump_tab[tc](x);
  }
}

/*
 * Value.C
 */
