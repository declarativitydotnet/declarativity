// -*- c-basic-offset: 2; related-file-name: "" -*-
/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Send simple message via UDP
 *
 */

#include "tuple.h"

#include "marshal.h"
#include "unmarshal.h"
#include "marshalField.h"
#include "unmarshalField.h"

#include "val_str.h"
#include "val_tuple.h"
#include "val_uint64.h"


void
testMarshaling()
{
  // Create a marshaling and an unmarshaling element.
  Marshal m("Marshal");
  Unmarshal u("Unmarshal");
  MarshalField mF("MarshalField", 0);
  UnmarshalField uF("UnmarshalField", 0);


  // Create a straight tuple
  // The input tuple
  TuplePtr flat = Tuple::mk();
  flat->append(Val_Str::mk("String"));
  flat->append(Val_UInt64::mk(13500975));
  flat->freeze();  

  
  // Marshal it
  TuplePtr result = m.simple_action(flat);

  ////////////////////////////////////////////////////////////
  // Sanity checks

  // It must have a single field
  if (result->size() != 1) {
    std::cerr << "** Marshalled tuple has different size from 1\n";
  } else {
    std::cout << "Marshalled tuple has size 1\n";
  }
  
  // Its field must be of type OPAQUE
  if ((*result)[0]->typeCode() != Value::OPAQUE) {
    std::cerr << "** Marshalled field is not OPAQUE 1\n";
  } else {
    std::cout << "Marshalled field is OPAQUE\n";
  }

  ////////////////////////////////////////////////////////////
  // Unmarshal

  TuplePtr reFlat = u.simple_action(result);

  // Compare the result to the original
  if (reFlat->compareTo(flat) != 0) {
    std::cerr << "** Marshalled/unmarshalled tuple does not match original\n";
  } else {
    std::cout << "Marshalled/unmarshalled tuple matches original\n";
  }


}


int main(int argc, char **argv)
{
  std::cout << "Marshal/Unmarshal tests, regardless of technology underneath\n";
  
  testMarshaling();
  return 0;
}
  

