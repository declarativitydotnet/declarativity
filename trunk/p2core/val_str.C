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

#include <arpc.h>

//
// Marshal a string
// 
void Val_Str::xdr_marshal( XDR *x ) 
{
  rpc_str<RPC_INFINITY> rs(s);
  rpc_traverse(x,rs);
}
  
/* 
 * End of file
 */
