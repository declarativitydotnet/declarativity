// -*- c-basic-offset: 2; related-file-name: "marshalField.h" -*-
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

#include "marshalField.h"

#include "val_opaque.h"
#include "xdrbuf.h"

MarshalField::MarshalField(string name,
                           unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

MarshalField::~MarshalField()
{
}

TuplePtr MarshalField::simple_action(TuplePtr p)
{
  // Take out the appropriate field
  ValuePtr value = (*p)[_fieldNo];

  // Does this field exist?
  if (value == NULL) {
    // Nope.  Return nothing
    return TuplePtr();
  } else {
    // Is this a field of type TUPLE?
    if (value->typeCode() == Value::TUPLE) {
      // Goodie. Marshal the field

      FdbufPtr fb(new Fdbuf());
      XDR xe;
      xdrfdbuf_create(&xe, fb.get(), false, XDR_ENCODE);
      value->xdr_marshal(&xe);
      xdr_destroy(&xe);
      
      // Now create the opaque
      ValuePtr marshalled = Val_Opaque::mk(fb);

      // Now create a tuple copy replacing the marshalled field
      TuplePtr newTuple = Tuple::mk();
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
