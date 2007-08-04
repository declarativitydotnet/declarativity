// -*- c-basic-offset: 2; related-file-name: "marshal.h" -*-
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
#include "marshal.h"

#include "val_opaque.h"
#include "fdbuf.h"
#include "xdrbuf.h"

Marshal::Marshal(string name)
  : Element(name, 1, 1)
{
}

Marshal::~Marshal()
{
}

TuplePtr Marshal::simple_action(TuplePtr p)
{
  // Taken straight from the tuples test.
  FdbufPtr fb(new Fdbuf());
  XDR xe;
  xdrfdbuf_create(&xe, fb.get(), false, XDR_ENCODE);
  p->xdr_marshal(&xe);
  xdr_destroy(&xe);

  // Now create a new tuple to host the opaque
  TuplePtr t = Tuple::mk();
  if (t == 0) {
    // Couldn't create one. Memory problems?
    log(Reporting::ERROR, -1, "Couldn't allocate new tuple");
    return TuplePtr();
  } else {
    // Stick the string into a tuple field and into the tuple
    t->append(Val_Opaque::mk(fb));
    t->freeze();
    return t;
  }
}
