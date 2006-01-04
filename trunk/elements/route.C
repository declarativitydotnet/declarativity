// -*- c-basic-offset: 2; related-file-name: "route.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "route.h"
#include "val_opaque.h"

Route::Route(string name, FdbufPtr destinationUio)
  : Element(name, 1, 1),
    _destination(Val_Opaque::mk(destinationUio))
{
  if (destinationUio->length() == 0) {
    // Got no destination. Bummer
    log(LoggerI::ERROR,
        -1,
        "Cannot create a route element without a destination address");
    exit(-1);
  }
}

Route::~Route()
{
}

TuplePtr Route::simple_action(TuplePtr p)
{
  // Get first tuple field
  ValuePtr firstP = (*p)[0];
  if (firstP == 0) {
    // No such field
    log(LoggerI::WARN,
        -1,
        "Input tuple has no first field");
    return TuplePtr();
  }
  ValuePtr first = firstP;

  // Is it a string?
  if (first->typeCode() != Value::OPAQUE) {
    // Can't route to something that isn't a marshalled opaque
    log(LoggerI::WARN,
        -1,
        "Input tuple's first field is not an opaque");
    return TuplePtr();
  }

  // Route and return new tuple
  TuplePtr routed = Tuple::mk();
  if (routed == 0) {
    // Couldn't create one. Memory problems?
    log(LoggerI::WARN,
        -1,
        "Couldn't allocate new tuple");

    return TuplePtr();
  } else {
    // Stick the destination address and the opaque into the new tuple
    routed->append(_destination);
    routed->append(first);
    routed->freeze();
    return routed;
  }
}
