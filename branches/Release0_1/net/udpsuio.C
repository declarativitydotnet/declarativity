// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
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
 * DESCRIPTION: Suio which retrieve sender's address for receiving
 * messages on a socket. 
 */

#include "udpsuio.h"
#include "loop.h"

int UdpSuio::inputfrom(int sd, struct sockaddr *from, socklen_t *fromlen) 
{
  clear();
  void *buf = getspace(MTU_SIZE);
  ssize_t sz = recvfrom(sd, buf, MTU_SIZE, 0, from, fromlen);
  if (sz >= MTU_SIZE) {
    warn << "UdpSuio: overly large packet received!\n";
    sz = MTU_SIZE;
  } else if (sz > 0) {
    pushiov( buf, sz);
  }
  return sz;
}
