// -*- c-basic-offset: 2; related-file-name: "print.h" -*-
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

#include "print.h"

Print::Print(string prefix)
  : Element(prefix, 1, 1),
    _prefix(prefix)
{
}

Print::~Print()
{
}

TuplePtr Print::simple_action(TuplePtr p)
{
  warn << "Print[" << _prefix << "]:  [" << p->toString() << "]\n";
  return p;
}
