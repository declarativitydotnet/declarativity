// -*- c-basic-offset: 2; related-file-name: "unmarshal.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "unmarshal.h"

#include "val_opaque.h"

Unmarshal::Unmarshal(str name)
  : Element(name, 1, 1)
{
}

Unmarshal::~Unmarshal()
{
}

TuplePtr Unmarshal::simple_action(TupleRef p)
{
  // Get first tuple field
  if (p->size() == 0) {
    log(LoggerI::WARN, -1, "Input tuple has no first field");
    return 0;
  }

  ref<suio> u = Val_Opaque::cast((*p)[0]);
  char *buf = suio_flatten(u);
  size_t sz = u->resid();
  xdrmem xd(buf,sz);
  TupleRef t = Tuple::xdr_unmarshal(&xd);
  xfree(buf);
  return t;
}
