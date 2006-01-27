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
 * DESCRIPTION: P2's concrete type system.  The base type for values.
 *
 */

#include "loop.h"
#include "value.h"
#define DEBUG_OFF
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
#include "val_time.h"
#include "val_id.h"

typedef ValuePtr (*_unmarshal_fn)( XDR *);

static _unmarshal_fn jump_tab[] = {
  Val_Null::xdr_unmarshal,
  Val_Str::xdr_unmarshal,
  Val_Int32::xdr_unmarshal,
  Val_UInt32::xdr_unmarshal,
  Val_Int64::xdr_unmarshal,
  Val_UInt64::xdr_unmarshal,
  Val_Double::xdr_unmarshal,
  Val_Opaque::xdr_unmarshal,
  Val_Tuple::xdr_unmarshal,
  Val_Time::xdr_unmarshal,
  Val_ID::xdr_unmarshal
};

//
// Marshalling
//

void Value::xdr_marshal( XDR *x ) 
{
  TRC_FN;
  uint32_t tc = typeCode();
  TRC("TypeCode is " << tc);
  xdr_uint32_t(x, &tc);
  xdr_marshal_subtype(x);
}

//
// Unmarshalling
//
ValuePtr Value::xdr_unmarshal(XDR *x)
{
  TRC_FN;
  uint32_t tc;
  xdr_uint32_t(x, &tc);
  TRC("TypeCode is " << tc);
  if ((unsigned) tc >= (sizeof(jump_tab)/sizeof(_unmarshal_fn))) {
    warn << "Unmarshalling: Bad typecode " << tc << "\n";
    return Val_Null::mk();
  } else {
    return jump_tab[tc](x);
  }
}

/*
 * Value.C
 */
