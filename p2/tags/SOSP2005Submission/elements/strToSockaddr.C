// -*- c-basic-offset: 2; related-file-name: "strToSockaddr.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "strToSockaddr.h"
#include "val_opaque.h"
#include "string.h"
#include "val_str.h"

StrToSockaddr::StrToSockaddr(str name,
                             unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

StrToSockaddr::~StrToSockaddr()
{
}

TuplePtr StrToSockaddr::simple_action(TupleRef p)
{
  // Get tuple field in question
  ValuePtr firstP = (*p)[_fieldNo];
  if (firstP == 0) {
    // No such field
    log(LoggerI::WARN,
        -1,
        "Input tuple has no field to translate");
    return 0;
  }
  ValueRef first = firstP;

  // Is it a string?
  if (first->typeCode() != Value::STR) {
    // Can't translate something that isn't a string
    log(LoggerI::WARN,
        -1,
        strbuf("Field to translate[") << first->toString()
        << "] is not a string");
    return 0;
  }

  // Split into address and port
  const char * theString = Val_Str::cast(first).cstr();
  char * theAtSign = strchr(theString, ':');
  if (theAtSign == NULL) {
    // Couldn't find the correct format
    log(LoggerI::WARN, -1, strbuf("Field to translate ")  << first->toString() << " is malformed");
    return 0;
  }
  str theAddress(theString, theAtSign - theString);
  str thePort(theAtSign + 1);
  int port = atoi(thePort);

  // Now construct the sockaddr
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_port = htons(port);
  inet_pton(AF_INET, theAddress.cstr(),
            &addr.sin_addr);
  ref< suio > addressUio = New refcounted< suio >();
  addressUio->copy(&addr, sizeof(addr));
  ValueRef sockaddr = Val_Opaque::mk(addressUio);
  

  // Finally, copy over the tuple
  TupleRef newTuple = Tuple::mk();
  for (unsigned field = 0;
       field < _fieldNo;
       field++) {
    newTuple->append((*p)[field]);
  }
  newTuple->append(sockaddr);
  for (unsigned field = _fieldNo + 1;
       field < p->size();
       field++) {
    newTuple->append((*p)[field]);
  }
  newTuple->freeze();
  return newTuple;
}
