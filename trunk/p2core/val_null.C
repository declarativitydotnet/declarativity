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
 * DESCRIPTION: P2's concrete type system: the NULL type.
 *
 */


#include "val_null.h"

//
// Singleton null value.
//
ValueRef Val_Null::singleton = New refcounted<Val_Null>();

//
// Marshalling and unmarshallng
//
void Val_Null::xdr_marshal( XDR *x ) 
{
  return;
}
