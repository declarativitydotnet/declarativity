// -*- c-basic-offset: 2; related-file-name: "noNullField.h" -*-
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

#include "noNullField.h"
#include "val_tuple.h"

NoNullField::NoNullField(string name, unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

NoNullField::~NoNullField()
{
}

TuplePtr NoNullField::simple_action(TuplePtr p)
{
  assert(p->size() > _fieldNo);

  // Only return p if the appropriate field is a tuple and has size
  // greater than 0
  if (Val_Tuple::cast((*p)[_fieldNo])->size() > 0) {
    return p;
  } else {
    return TuplePtr();
  }
}
