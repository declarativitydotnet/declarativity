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

Marshal::Marshal()
  : Element(1, 1)
{
}

Marshal::~Marshal()
{
}

TuplePtr Marshal::simple_action(TupleRef p)
{
  // Taken straight from the tuples test.
  xdrsuio xe;
  p->xdr_marshal(&xe);
  const char *buf = suio_flatten(xe.uio());
  size_t sz = xe.uio()->resid();
  str s = armor64(buf, sz);

  // Now create a new tuple to host this string
  TuplePtr marshalledTuple = Tuple::mk();
  if (marshalledTuple == 0) {
    // Couldn't create one. Memory problems?
    log(LoggerI::WARN,
        -1,
        "Couldn't allocate new tuple");

    return 0;
  } else {
    // Stick the string into a tuple field and into the tuple
    marshalledTuple->append(New refcounted<TupleField>(s));
    marshalledTuple->freeze();
    return marshalledTuple;
  }
}
