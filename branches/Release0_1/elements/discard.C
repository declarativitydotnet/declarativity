// -*- c-basic-offset: 2; related-file-name: "discard.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element discarding all tuples pushed into it
 */

#include "discard.h"

Discard::Discard(string name) :
  Element(name, 1,0)
{
}

int Discard::push(int port, TuplePtr, b_cbv cb)
{
  // Send as many more tuples as you want
  return 1;
}
