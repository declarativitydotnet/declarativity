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
 * DESCRIPTION: Tuple fields and Tuple implementations
 *
 */

#include "tuple.h"
#include <assert.h>


//
// XDR function for null values.  Not a lot to say really.
//
static bool_t xdr_null_t(XDR *x, void *p)
{
  // Nothing to marshal..
  return true;
}

//
// Tuple field information: how big?  What's it called?  How do you
// marshal it? 
//
typedef bool_t (*xdrproc_t)(XDR *, void *);
struct tinfo {
  size_t size;
  const char *name;
  xdrproc_t xdrproc;
};
static const tinfo tinfos[] = {
  { 0, "null", xdr_null_t },
  { sizeof(int32_t), "int32_t", xdr_int32_t },
  { sizeof(u_int32_t), "u_int32_t", xdr_u_int32_t },
  { sizeof(int64_t), "int64_t", xdr_int64_t },
  { sizeof(u_int64_t), "u_int64_t", xdr_u_int64_t },
  { 0, "string", NULL },
  { sizeof(double), "double", (xdrproc_t)xdr_double },
  { 0, "invalid", NULL }
};

//
// Why the explicit marshalling code, and not just a type definition
// in RPCC?  Well, it's tedious to do unions in XDR, and there's not a
// great deal needed here.  If tuples become a lot more complex
// (heaven forbid), we might move to a Tuple.x file.  We might also
// want to recast these functions as an rpc_traverse template
// instantiating, as DM presumably would have done. 
//

// 
// Serialize a Tuple Field into an XDR
//
void TupleField::xdr_marshal( XDR *x )
{
  // We don't bother dimensioning the uio, because we're assuming this
  // is done as an optimization at the Tuple level.  In any case it
  // won't affect correctness.
  xdr_u_int32_t( x, &t );
  if ( t >= INVALID ) {
    fatal << "TupleField::xdr_marshal: bad type code of " << t << "\n";
  } else if ( t == STRING ) {
    rpc_str<RPC_INFINITY> rs(s);
    rpc_traverse(x,rs);
  } else {
    (tinfos[t].xdrproc)(x, &i32);
  }
}

// 
// Deserialize a Tuple Field from an XDR buffer
//
TupleField *TupleField::xdr_unmarshal( XDR *x )
{
  TupleField *tf = New TupleField();
  xdr_u_int32_t( x, &(tf->t) );
  if ( tf->t >= INVALID ) {
    throw TypeError();
  } else if ( tf->t == STRING ) {
    // Note that this looks like a yucky double copy, but at least the
    // string data itself isn't copied (since rpc_str <: str). 
    rpc_str<RPC_INFINITY> rs;
    rpc_traverse(x,rs);
    tf->s = rs;
  } else {
    (tinfos[tf->t].xdrproc)(x, &tf->i32);
  }
  return tf;
}

//
// Serialize a Tuple into an XDR
//
void Tuple::xdr_marshal( XDR *x ) {
  assert(finalized);
  assert(sizeof(size_t) == sizeof(u_int32_t));
  // Tuple size overall
  size_t sz = fields.size();
  xdr_u_int32_t(x, &sz );
  // Marshal the fields
  for(size_t i=0; i < fields.size(); i++) {
    fields[i].xdr_marshal(x);
  };
}

//
// Deserialize a Tuple from an XDR
//
Tuple *Tuple::xdr_unmarshal( XDR *x ) {
  Tuple *t = New Tuple();
  assert(sizeof(size_t) == sizeof(u_int32_t));
  // Tuple size overall
  size_t sz;
  xdr_u_int32_t(x, &sz );
  // Marshal the fields
  for(size_t i=0; i < sz; i++) {
    TupleField::xdr_unmarshal(x);
  }
  return t;
}
