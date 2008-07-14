// -*- c-basic-offset: 2; related-file-name: "unmarshalField.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "unmarshalField.h"

#include "val_opaque.h"
#include "val_tuple.h"

UnmarshalField::UnmarshalField(str name,
                               unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

UnmarshalField::~UnmarshalField()
{
}

TuplePtr UnmarshalField::simple_action(TupleRef p)
{
  // Get the field in question
  ValuePtr value = (*p)[_fieldNo];

  // Does this field exist?
  if (value == NULL) {
    // Nope.  Return nothing
    return 0;
  } else {
    // Is this a field of type OPAQUE?
    if (value->typeCode() == Value::OPAQUE) {
      // Goodie. Unmarshal the field

      ref<suio> u = Val_Opaque::cast(value);
      char *buf = suio_flatten(u);
      size_t sz = u->resid();
      xdrmem xd(buf,sz);
      ValueRef unmarshalled = Value::xdr_unmarshal(&xd);
      xfree(buf);

      // Now create a tuple copy replacing the unmarshalled field
      TupleRef newTuple = Tuple::mk();
      for (unsigned field = 0;
           field < _fieldNo;
           field++) {
        newTuple->append((*p)[field]);
      }
      newTuple->append(unmarshalled);
      for (unsigned field = _fieldNo + 1;
           field < p->size();
           field++) {
        newTuple->append((*p)[field]);
      }
      newTuple->freeze();
      return newTuple;
    } else {
      // Numbered field is un-unmarshallable.  Just return the same
      // tuple and log a warning
      log(LoggerI::WARN, -1, "Cannot unmarshal a non-opaque field");
      return p;
    }
  }
  

}
