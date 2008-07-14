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

#include "tupleInjector.h"
#include "val_str.h"


DEFINE_ELEMENT_INITS(TupleInjector, "TupleInjector")


TupleInjector::TupleInjector(string name)
  : Element(name, 0, 1)
{
}


/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 */
TupleInjector::TupleInjector(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 0, 1)
{
}


int
TupleInjector::tuple(TuplePtr tp,
                     b_cbv cb)
{
  ELEM_WORDY("Got tuple injection "
             << tp->toString());
  return output(0)->push(tp, cb);
}
