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
 * DESCRIPTION: An XDR stream implementation for P2 fdbufs
 *
 */

#include "xdrbuf.h"

#include "fdbuf.h"
#include <assert.h>

//
// Forward XDR method declarations
//
static bool_t	fd_getlong(XDR *xdrs, long *lp);
static bool_t	fd_putlong(XDR *xdrs, const long *lp);
static bool_t	fd_getbytes(XDR *xdrs, caddr_t addr, u_int len);
static bool_t	fd_putbytes(XDR *xdrs, const char *addr, u_int len);
static u_int	fd_getpostn(const XDR *xdrs);
static bool_t	fd_setpostn(XDR *xdrs, u_int pos);
static int32_t *fd_inline(XDR *xdrs, u_int len);
static void	fd_destroy(XDR *xdrs);
static bool_t	fd_getint32(XDR *xdrs, int32_t *lp);
static bool_t	fd_putint32(XDR *xdrs, const int32_t *lp);

//
// XDR ops table
//
static XDR::xdr_ops ops = {
  fd_getlong,
  fd_putlong,
  fd_getbytes,
  fd_putbytes,
  fd_getpostn,
  fd_setpostn,
  fd_inline,
  fd_destroy,
  fd_getint32,
  fd_putint32
};

//
// Convert to an Fdbuf...
//
static inline Fdbuf *fdb(const XDR *xdrs)
{
  assert(xdrs->x_ops == &ops);
  return (Fdbuf *)(xdrs->x_private);
}

//
// Functions
//
static bool_t fd_getlong(XDR *xdrs, long *lp)
{
  if (fdb(xdrs)->length() > sizeof(*lp)) {
    fdb(xdrs)->align_read();
    *lp = ntohl(fdb(xdrs)->pop_uint32());
    return true;
  } else {
    return false;
  }
}
static bool_t fd_putlong(XDR *xdrs, const long *lp)
{ 
  fdb(xdrs)->align_write();
  fdb(xdrs)->push_back(htonl(*lp));
  return true;
}
static bool_t fd_getbytes(XDR *xdrs, caddr_t addr, u_int len)
{
  return fdb(xdrs)->pop_bytes(addr, len);
}
static bool_t fd_putbytes(XDR *xdrs, const char *addr, u_int len)
{
  fdb(xdrs)->push_back(addr, len);
  return true;
}
static u_int fd_getpostn(const XDR *xdrs)
{
  if (xdrs->x_op == XDR_ENCODE) {
    return fdb(xdrs)->length();
  } else {
    return fdb(xdrs)->removed();
  }
}
static bool_t fd_setpostn(XDR *xdrs, u_int pos)
{
  return false;
}
static int32_t *fd_inline(XDR *xdrs, u_int len)
{
  // return (int32_t *)(fdb(xdrs)->raw_inline(len));
  // Probably unnecessary to implement, but we need to say exactly
  // what our requirements here are in terms of word-alignment...
  return NULL;
}
static void fd_destroy(XDR *xdrs)
{
  if (xdrs->x_handy) {
    delete fdb(xdrs);
  }
}
static bool_t	fd_getint32(XDR *xdrs, int32_t *lp)
{
  return fd_getlong(xdrs, (long int *)lp);
}
static bool_t	fd_putint32(XDR *xdrs, const int32_t *lp)
{
  return fd_putlong(xdrs, (long int *)lp);
}

//
// Initialize an XDR to use an Fdbuf.  If 'take' is true, the fdbuf
// will be destroyed when the XDR is destroyed. 
//
void xdrfdbuf_create(XDR *xdrs, Fdbuf *fdb, bool take, enum xdr_op op)
{
  xdrs->x_op = op;
  xdrs->x_ops = &ops;
  xdrs->x_private = (caddr_t)fdb;
  xdrs->x_handy = take;
}

