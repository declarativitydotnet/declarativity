// -*- c-basic-offset: 2; related-file-name: "unmarshal.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "unmarshal.h"

Unmarshal::Unmarshal()
  : Element(1, 1)
{
}

Unmarshal::~Unmarshal()
{
}

TuplePtr Unmarshal::simple_action(TupleRef p)
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

  // Dearmor the string to a char*

  // XXX What if the string is not armored?
  str dearmored = dearmor64(first->as_s());
  const char * buf = dearmored.cstr();
  size_t sz = dearmored.len();
  if (buf == 0) {
    log(LoggerI::WARN,
        -1,
        "String to unmarshal is null");
  }
  
  xdrmem xd(buf, sz);
  TupleRef t = Tuple::xdr_unmarshal(&xd);

  // Return this one
  return t;
}
