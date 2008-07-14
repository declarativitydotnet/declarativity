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

#ifdef WIN32
#include "p2_win32.h"
#endif // WIN32

#include "unmarshalField.h"

#include "val_opaque.h"
#include "val_tuple.h"
//#include "xdrbuf.h"
// the boost serialization implementer claims text is not much more expensive than portable binary
#include <boost/archive/text_iarchive.hpp>
#include "val_str.h"
#include "val_uint32.h"
#include "val_list.h"

DEFINE_ELEMENT_INITS(UnmarshalField, "UnmarshalField")

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

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_UInt32: Field number.
 * OR
 * 3. Val_List:   Field numbers.
 */
UnmarshalField::UnmarshalField(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1)
{
  if ((*args)[3]->typeCode() == Value::LIST) {
    ListPtr fieldNos = Val_List::cast((*args)[3]);
    for (ValPtrList::const_iterator i = fieldNos->begin();
         i != fieldNos->end(); i++)
      _fieldNos.push_back(Val_UInt32::cast(*i));
  }
  else {
    _fieldNos.push_back(Val_UInt32::cast((*args)[3]));
  }
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
    else if (value->typeCode() == Value::P2_OPAQUE) {
      // Goodie. Unmarshal the field
      FdbufPtr fb = Val_Opaque::cast(value);
	  std::stringstream ss(fb->str());
	  boost::archive::text_iarchive xd(ss);
		  //      xdrfdbuf_create(&xd, fb.get(), false, XDR_DECODE);
      ValuePtr unmarshalled = Value::unmarshal(&xd);
//      xdr_destroy(&xd);

      newTuple->append(unmarshalled);
    } else {
      // Numbered field is un-unmarshallable.  Just return the same
      // tuple and log a warning
      ELEM_WARN("Cannot unmarshal a non-opaque field");
      return p;
    }
  }
  newTuple->freeze();
  return newTuple;
}
