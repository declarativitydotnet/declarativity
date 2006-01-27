// -*- c-basic-offset: 2; related-file-name: "unmarshal.h" -*-
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
 */

#include "unmarshal.h"

#include "val_opaque.h"
#include "xdrbuf.h"

Unmarshal::Unmarshal(string name)
  : Element(name, 1, 1)
{
}

Unmarshal::~Unmarshal()
{
}

TuplePtr Unmarshal::simple_action(TuplePtr p)
{
  // Get first tuple field
  if (p->size() == 0) {
    log(LoggerI::WARN, -1, "Input tuple has no first field");
    return TuplePtr();
  }

  XDR xd;
  FdbufPtr fb = Val_Opaque::cast((*p)[0]);
  xdrfdbuf_create(&xd, fb.get(), false, XDR_DECODE);
  TuplePtr t = Tuple::xdr_unmarshal(&xd);
  xdr_destroy(&xd);
  return t;
}
