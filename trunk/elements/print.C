// -*- c-basic-offset: 2; related-file-name: "print.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "print.h"

#include <iostream>

Print::Print()
  : Element(1, 1)
{
}

Print::~Print()
{
}

int Print::push(int port, TupleRef t, cbv cb)
{
  return 1;
}

TuplePtr Print::pull(int port, cbv)
{
  return 0;
}
