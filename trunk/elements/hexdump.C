// -*- c-basic-offset: 2; related-file-name: "hexdump.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "hexdump.h"

Hexdump::Hexdump()
  : Element(1, 1)
{
}

Hexdump::~Hexdump()
{
}

TuplePtr Hexdump::simple_action(TupleRef p)
{
  // Get first tuple field
  TupleFieldPtr first = (*p)[0];
  if (first == 0) {
    // No such field
    log(LoggerI::WARN,
        -1,
        "Input tuple has no first field");
    return 0;
  }

  // Is it a string?
  if (first->get_type() != TupleField::STRING) {
    // Can't hexdump a string
    log(LoggerI::WARN,
        -1,
        "Input tuple's first field is not a string");
    return 0;
  }

  // Hexdump and return new tuple
  str firstStr = first->as_s();
  str s = strbuf() << hexdump(firstStr, firstStr.len());
  TuplePtr hexedTuple = Tuple::mk();
  if (hexedTuple == 0) {
    // Couldn't create one. Memory problems?
    log(LoggerI::WARN,
        -1,
        "Couldn't allocate new tuple");

    return 0;
  } else {
    // Stick the string into a tuple field and into the tuple
    hexedTuple->append(New refcounted<TupleField>(s));
    hexedTuple->freeze();
    return hexedTuple;
  }
}
