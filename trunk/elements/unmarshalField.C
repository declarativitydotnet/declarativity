// -*- c-basic-offset: 2; related-file-name: "unmarshalField.h" -*-
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

#include "unmarshalField.h"

#include "val_opaque.h"
#include "val_tuple.h"
#include "xdrbuf.h"

UnmarshalField::UnmarshalField(string name,
                               unsigned fieldNo)
  : Element(name, 1, 1)
{
  _fieldNos.push_back(fieldNo);
}

UnmarshalField::UnmarshalField(string name,
                               std::vector<unsigned> fieldNos)
  : Element(name, 1, 1), _fieldNos(fieldNos)
{
}


UnmarshalField::~UnmarshalField()
{
}

TuplePtr UnmarshalField::simple_action(TuplePtr p)
{
  TuplePtr newTuple = Tuple::mk();
  // Take out the appropriate field
  for (unsigned field = 0; field < p->size(); field++) {
    ValuePtr value = (*p)[field];

    if (std::find(_fieldNos.begin(), _fieldNos.end(), field) 
        == _fieldNos.end()) {
      newTuple->append((*p)[field]);	// Just add it
    }
    else if (value == NULL) {
      // Nope.  Return nothing
      return TuplePtr();
    } 
    // Is this a field of type OPAQUE?
    else if (value->typeCode() == Value::OPAQUE) {
      // Goodie. Unmarshal the field
      FdbufPtr fb = Val_Opaque::cast(value);
      XDR xd;
      xdrfdbuf_create(&xd, fb.get(), false, XDR_DECODE);
      ValuePtr unmarshalled = Value::xdr_unmarshal(&xd);
      xdr_destroy(&xd);

      newTuple->append(unmarshalled);
    } else {
      // Numbered field is un-unmarshallable.  Just return the same
      // tuple and log a warning
      log(Reporting::WARN, -1, "Cannot unmarshal a non-opaque field");
      return p;
    }
  }
  newTuple->freeze();
  return newTuple;
}
