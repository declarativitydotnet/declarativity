/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "noNullSignal.h"
#include "val_str.h"

DEFINE_ELEMENT_INITS(NoNullSignal, "NoNullSignal");

NoNullSignal::NoNullSignal(string name)
  : Element(name, 1, 2)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 */
NoNullSignal::NoNullSignal(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 2)
{
}

NoNullSignal::~NoNullSignal()
{
}

TuplePtr NoNullSignal::simple_action(TuplePtr p)
{
  // Only return p if it has size greater than 0
  if (p->size() > 0) {
    return p;
  } else {
    output(1)->push(p, 0);
    return TuplePtr();
  }
}
