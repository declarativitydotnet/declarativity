/*
 * @(#)$Id$
 *
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
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
static u_int    fd_getpostn(
#ifndef __APPLE__
                            const
#endif
                            XDR *xdrs);
static bool_t	fd_setpostn(XDR *xdrs, u_int pos);
static int32_t *fd_inline(XDR *xdrs, u_int len);
static void	fd_destroy(XDR *xdrs);
static bool_t	fd_getint32(XDR *xdrs, int32_t *lp);
static bool_t	fd_putint32(XDR *xdrs, const int32_t *lp);
static bool_t	fd_control(XDR *xdrs, int c, void *ch);

//
// XDR ops table
//
#ifdef __APPLE__
static XDR::xdr_ops ops = {
  fd_getlong,
  fd_putlong,
  fd_getbytes,
  fd_putbytes,
  fd_getpostn,
  fd_setpostn,
  fd_inline,
  fd_destroy,
  fd_control
};
#else
//
// XDR ops table
//
static XDR::xdr_ops ops = (const XDR::xdr_ops){
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
#endif

//
// Convert to an Fdbuf...
//
static inline Fdbuf *fdb(const XDR *xdrs)
{
  assert(xdrs->x_ops == &ops);
  return (Fdbuf *)(xdrs->x_private);
}





////////////////////////////////////////////////////////////
// Ops Table Functions
////////////////////////////////////////////////////////////

static bool_t
fd_getlong(XDR *xdrs, long *lp)
{
  return fd_getint32(xdrs, (int32_t*)lp);
}


static bool_t
fd_putlong(XDR *xdrs, const long *lp)
{ 
  return fd_putint32(xdrs, (int32_t*)lp);
}


static bool_t
fd_getbytes(XDR *xdrs, caddr_t addr, u_int len)
{
  bool_t result = fdb(xdrs)->pop_bytes(addr, len);
  return result;
}

static bool_t
fd_putbytes(XDR *xdrs, const char *addr, u_int len)
{
  fdb(xdrs)->push_bytes(addr, len);
  return true;
}
/** Removed const for MAC build */
static u_int fd_getpostn(
#ifndef __APPLE__
const
#endif
XDR *xdrs)
{
  if (xdrs->x_op == XDR_ENCODE) {
    return fdb(xdrs)->length();
  } else {
    return fdb(xdrs)->removed();
  }
}


static bool_t
fd_setpostn(XDR *xdrs, u_int pos)
{
  return false;
}


static int32_t
*fd_inline(XDR *xdrs, u_int len)
{
  // return (int32_t *)(fdb(xdrs)->raw_inline(len));
  // Probably unnecessary to implement, but we need to say exactly
  // what our requirements here are in terms of word-alignment...
  return NULL;
}


static void
fd_destroy(XDR *xdrs)
{
  if (xdrs->x_handy) {
    delete fdb(xdrs);
  }
}


static bool_t
fd_getint32(XDR *xdrs, int32_t *lp)
{
  if (fdb(xdrs)->length() >= sizeof(int32_t)) {
    fdb(xdrs)->align_read();
    *lp = (int32_t) ntohl(fdb(xdrs)->pop_uint32());
    return true;
  } else {
    return false;
  }
}


static bool_t
fd_putint32(XDR *xdrs, const int32_t *lp)
{
  fdb(xdrs)->align_write();
  fdb(xdrs)->push_uint32(htonl((uint32_t)*lp));
  return true;
}

/** Function specific to the Mac version of xdr_ops */
static bool_t
fd_control(XDR *xdrs, int c, void *ch)
{
  return false;
}

////////////////////////////////////////////////////////////
// End of Ops Table Functions
////////////////////////////////////////////////////////////




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

