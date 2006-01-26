/*
 * @(#)$Id$
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: An XDR stream implementation for P2 fdbufs
 *
 */

#ifndef __XDRBUF_H__
#define __XDRBUF_H__

#include "fdbuf.h"
#include <rpc/xdr.h>

//
// Initialize an XDR to use an Fdbuf.  If 'take' is true, the fdbuf
// will be destroyed when the XDR is destroyed. 
//
extern void xdrfdbuf_create(XDR *xdrs, Fdbuf *fdb, bool take, enum xdr_op op);

#endif // __XDRBUF_H__
