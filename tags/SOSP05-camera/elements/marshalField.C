// -*- c-basic-offset: 2; related-file-name: "marshalField.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "marshalField.h"
#include "xdr_suio.h"

#include "val_opaque.h"

MarshalField::MarshalField(str name,
                           unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

MarshalField::~MarshalField()
{
}

TuplePtr MarshalField::simple_action(TupleRef p)
{
  // Take out the appropriate field
  ValuePtr value = (*p)[_fieldNo];

  // Does this field exist?
  if (value == NULL) {
    // Nope.  Return nothing
    return 0;
  } else {
    // Is this a field of type TUPLE?
    if (value->typeCode() == Value::TUPLE) {
      // Goodie. Marshal the field

      xdrsuio xe;
      value->xdr_marshal(&xe);
      ref<suio> uio = New refcounted<suio>();
      uio->take(xsuio(&xe));
      
      // Now create the opaque
      ValueRef marshalled = Val_Opaque::mk(uio);

      // Now create a tuple copy replacing the marshalled field
      TupleRef newTuple = Tuple::mk();
      for (unsigned field = 0;
           field < _fieldNo;
           field++) {
        newTuple->append((*p)[field]);
      }
      newTuple->append(marshalled);
      for (unsigned field = _fieldNo + 1;
           field < p->size();
           field++) {
        newTuple->append((*p)[field]);
      }
      newTuple->freeze();
      return newTuple;
    } else {
      // Numbered field is unmarshallable.  Just return the same tuple
      // and log a warning
      log(LoggerI::WARN, -1, "Cannot marshal a non-tuple field");
      return p;
    }
  }
}
