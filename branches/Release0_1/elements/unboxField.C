// -*- c-basic-offset: 2; related-file-name: "unmarshalField.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "unboxField.h"

#include "val_opaque.h"
#include "val_tuple.h"

UnboxField::UnboxField(string name, unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

UnboxField::~UnboxField()
{
}

TuplePtr UnboxField::simple_action(TuplePtr p)
{
  // Get the field in question
  ValuePtr value = (*p)[_fieldNo];

  // Does this field exist?
  if (!value) {
    // Nope.  Return nothing
    return TuplePtr();
  } 
  if (value->typeCode() != Value::TUPLE) {
    return TuplePtr(); // bye bye
  }

  TuplePtr tup = Val_Tuple::cast(value);
  return tup;
}
