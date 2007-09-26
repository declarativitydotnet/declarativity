// -*- c-basic-offset: 2; related-file-name: "strToSockaddr.h" -*-
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

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "strToSockaddr.h"
#include "val_opaque.h"
#include "string.h"
#include "val_str.h"
#include "val_uint32.h"

DEFINE_ELEMENT_INITS(StrToSockaddr, "StrToSockaddr")

StrToSockaddr::StrToSockaddr(string name, unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
{
}

/**
 * Generic constructor.
 * Arguments:
 * 2. Val_Str:    Element Name.
 * 3. Val_UInt32: Field number to convert.
 */
StrToSockaddr::StrToSockaddr(TuplePtr args)
  : Element(Val_Str::cast((*args)[2]), 1, 1),
    _fieldNo(Val_UInt32::cast((*args)[3]))
{
}

StrToSockaddr::~StrToSockaddr()
{
}

TuplePtr StrToSockaddr::simple_action(TuplePtr p)
{
  // Get tuple field in question
  ValuePtr firstP = (*p)[_fieldNo];
  if (firstP == 0) {
    // No such field
    ELEM_WARN("Input tuple has no field to translate");
    return TuplePtr();
  }
  ValuePtr first = firstP;

  // Is it a string?
  if (first->typeCode() != Value::STR) {
    // Can't translate something that isn't a string
    ELEM_WARN("Field to translate["
              << first->toString()
              << "] is not a string");
    return TuplePtr();
  }

  // Split into address and port
  const char * theString = Val_Str::cast(first).c_str();
  char * theAtSign = strchr(theString, ':');
  if (theAtSign == NULL) {
    // Couldn't find the correct format
    ELEM_WARN("Field to translate "
              << first->toString()
              << " is malformed");
    return TuplePtr();
  }
  string theAddress(theString, theAtSign - theString);
  struct hostent *host = gethostbyname(theAddress.c_str());
  if (host != NULL) {
    theAddress = inet_ntoa(*((struct in_addr*)host->h_addr));
  }
  string thePort(theAtSign + 1);
  int port = atoi(thePort.c_str());

  // Now construct the sockaddr
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_len = sizeof(sockaddr_in);
#endif // HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  inet_pton(AF_INET, theAddress.c_str(), &addr.sin_addr);
  FdbufPtr addressUio(new Fdbuf());
  addressUio->push_bytes((char*)&addr, sizeof(addr));
  ValuePtr sockaddr = Val_Opaque::mk(addressUio);
  

  // Finally, copy over the tuple
  TuplePtr newTuple = Tuple::mk();
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
