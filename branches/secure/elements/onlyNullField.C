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

#include "onlyNullField.h"
#include "val_tuple.h"
#include "val_str.h"
#include "val_int64.h"

DEFINE_ELEMENT_INITS(OnlyNullField, "OnlyNullField");

OnlyNullField::OnlyNullField(string name, unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_UInt32: The field number that should not be null.
 */
OnlyNullField::OnlyNullField(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _fieldNo(Val_Int64::cast((*args)[3]))
{
}

OnlyNullField::~OnlyNullField()
{
}

TuplePtr OnlyNullField::simple_action(TuplePtr p)
{
  assert(p->size() > _fieldNo);

  // Only return p if the appropriate field is a tuple and has size
  // greater than 0
  if (Val_Tuple::cast((*p)[_fieldNo])->size() == 0) {
    return p;
  } else {
    return TuplePtr();
  }
}
