// -*- c-basic-offset: 2; related-file-name: "noNull.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "noNull.h"

NoNull::NoNull(string name)
  : Element(name, 1, 1)
{
}

NoNull::~NoNull()
{
}

TuplePtr NoNull::simple_action(TuplePtr p)
{
  // Only return p if it has size greater than 0
  if (p->size() > 0) {
    return p;
  } else {
    return TuplePtr();
  }
}
