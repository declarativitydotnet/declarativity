// -*- c-basic-offset: 2; related-file-name: "dupElim.h" -*-
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

#include "dupElim.h"
#include "val_str.h"
#include "val_int64.h"

DEFINE_ELEMENT_INITS(DupElim, "DupElim");

DupElim::DupElim(string name)
  : Element(name, 1, 1)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str: Element Name.
 */
DupElim::DupElim(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1)
{
}

TuplePtr DupElim::simple_action(TuplePtr p)
{
  // Attempt to insert tuple
  std::pair< std::set< TuplePtr >::iterator, bool > result = _table.insert(p);

  // Did we succeed?
  if (result.second) {
    // Yup, the tuple is inserted
    return p;
  } else {
    // No, another one was there
    return TuplePtr();
  }
}
