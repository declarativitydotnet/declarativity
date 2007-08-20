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

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32

#include "marshalField.h"

#include "val_opaque.h"
//#include "xdrbuf.h"
// the boost serialization implementer claims text is not much more expensive than portable binary
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
#include "val_str.h"
#include "val_uint32.h"
#include "val_list.h"

DEFINE_ELEMENT_INITS(MarshalField, "MarshalField");

MarshalField::MarshalField(string name,
                           unsigned fieldNo)
  : Element(name, 1, 1)
{
  _fieldNos.push_back(fieldNo);
}

MarshalField::MarshalField(string name, std::vector<unsigned> fieldNos)
  : Element(name, 1, 1), _fieldNos(fieldNos)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_UInt32: The field number to be marshalled.
 * OR
 * 3. Val_List:   The field numbers to be marshalled.
 */
MarshalField::MarshalField(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1)
{
  if ((*args)[3]->typeCode() == Value::LIST) {
    ListPtr fields = Val_List::cast((*args)[3]);
    for (ValPtrList::const_iterator i = fields->begin();
         i != fields->end(); i++)
      _fieldNos.push_back(Val_UInt32::cast(*i));
  }
  else {
    _fieldNos.push_back(Val_UInt32::cast((*args)[3]));
  }
}

MarshalField::~MarshalField()
{
}

TuplePtr MarshalField::simple_action(TuplePtr p)
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
    else if (value->typeCode() == Value::P2_OPAQUE) {
      newTuple->append((*p)[field]);	// Just add it
    }
    else if (value->typeCode() == Value::TUPLE) {
	  // Goodie. Marshal the field
		std::stringstream outstr;
		boost::archive::text_oarchive xe(outstr);
      FdbufPtr fb(new Fdbuf());
      // xdrfdbuf_create(&xe, fb.get(), false, XDR_ENCODE);
      value->marshal(&xe);
     // xdr_destroy(&xe);
      
      // Now create the opaque
	  fb->pushString(outstr.rdbuf()->str());
      ValuePtr marshalled = Val_Opaque::mk(fb);

      newTuple->append(marshalled);
    }  
    else {
      // Numbered field is unmarshallable.  Just return the same tuple
      // and log a warning
      ELEM_WARN("Cannot marshal a non-tuple field");
      return p;
    }
  }
  newTuple->freeze();
  return newTuple;
}
