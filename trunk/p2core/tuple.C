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
// Serialize a Tuple into an XDR
//
void Tuple::xdr_marshal( XDR *x ) 
{
  assert(frozen);
  assert(sizeof(size_t) == sizeof(u_int32_t));
  // Tuple size overall
  size_t sz = fields.size();
  xdr_u_int32_t(x, &sz );
  // Marshal the fields
  for(size_t i=0; i < fields.size(); i++) {
    fields[i]->xdr_marshal(x);
  };
}

//
// Deserialize a Tuple from an XDR
//
ref<Tuple> Tuple::xdr_unmarshal( XDR *x ) 
{
  TupleRef t = Tuple::mk();
  assert(sizeof(size_t) == sizeof(u_int32_t));
  // Tuple size overall
  size_t sz;
  xdr_u_int32_t(x, &sz );
  // Marshal the fields
  for(size_t i=0; i < sz; i++) {
    t->append(Value::xdr_unmarshal(x));
  }
  return t;
}

//
// Format as a string
//
str Tuple::toString() const 
{
  strbuf sb;

  sb << "<";
  for(size_t i=0; i < fields.size(); i++) {
    sb << fields[i]->toString();
    if (i != fields.size() - 1) {
      sb << ", ";
    }
  }
  sb << ">";
  return str(sb);
}

