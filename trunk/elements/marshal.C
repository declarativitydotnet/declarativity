// -*- c-basic-offset: 2; related-file-name: "marshal.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */
#include "marshal.h"
#include "xdr_suio.h"

#include "val_opaque.h"

Marshal::Marshal(str name)
  : Element(name, 1, 1)
{
}

Marshal::~Marshal()
{
}

TuplePtr Marshal::simple_action(TuplePtr p)
{
  // Taken straight from the tuples test.
  xdrsuio xe;
  p->xdr_marshal(&xe);
  ref<suio> uio = New refcounted<suio>();
  uio->take(xsuio(&xe));

  // Now create a new tuple to host the opaque
  TuplePtr t = Tuple::mk();
  if (t == 0) {
    // Couldn't create one. Memory problems?
    log(LoggerI::ERROR, -1, "Couldn't allocate new tuple");
    return TuplePtr();
  } else {
    // Stick the string into a tuple field and into the tuple
    t->append(Val_Opaque::mk(uio));
    t->freeze();
    return t;
  }
}
