// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which simply prints any tuple pushed to it.
 * It's always ready. 
 * 
 */

#include "pushprint.h"
#include <tuple.h>

#include <iostream>

PushPrint::PushPrint() : Element(1,0)
{
}

int PushPrint::push(int port, TupleRef t, cbv cb)
{
  std::cout << "PushPrint: " << (t->toString()) << "\n";
  return 1;
}
