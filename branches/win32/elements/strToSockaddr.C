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

#ifdef WIN32
#include "p2_win32.h"
#include <winsock2.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // WIN32

#include "strToSockaddr.h"
#include "val_opaque.h"
#include "string.h"
#include "val_str.h"

StrToSockaddr::StrToSockaddr(string name, unsigned fieldNo)
  : Element(name, 1, 1),
    _fieldNo(fieldNo)
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
    log(Reporting::WARN,
        -1,
        "Input tuple has no field to translate");
    return TuplePtr();
  }
  ValuePtr first = firstP;

  // Is it a string?
  if (first->typeCode() != Value::STR) {
    // Can't translate something that isn't a string
    log(Reporting::WARN, -1,
        string("Field to translate[") + first->toString() + "] is not a string");
    return TuplePtr();
  }

  // Split into address and port
#ifdef WIN32
  string str = Val_Str::cast(first);
  const char * theString = str.c_str();
#else
  const char * theString = Val_Str::cast(first).c_str();
#endif
  const char * theAtSign = strchr(theString, ':');
  if (theAtSign == NULL) {
    // Couldn't find the correct format
    ELEM_WARN("Field to translate "
              << first->toString()
              << " is malformed");
    return TuplePtr();
  }
  string theAddress(theString, theAtSign - theString);
#ifdef WIN32
  LPHOSTENT 
#else
  struct hostent *
#endif // WIN32
	         host = gethostbyname(theAddress.c_str());
  if (host != NULL) {
    theAddress = inet_ntoa(*((struct in_addr*)host->h_addr));
  }
  string thePort(theAtSign + 1);
  int port = atoi(thePort.c_str());

  // Now construct the sockaddr
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_len = sizeof(sockaddr_in);
#endif // HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  //inet_pton(AF_INET, theAddress.c_str(), &addr.sin_addr);
#ifdef WIN32
  addr.sin_addr = *((LPIN_ADDR)*host->h_addr_list);
#else
  addr.sin_addr.s_addr = inet_addr(theAddress.c_str());
#endif // WIN32

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
