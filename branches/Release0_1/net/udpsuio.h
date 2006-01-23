// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element-pair for a UDP socket
 */

#ifndef __UDPSUIO_H__
#define __UDPSUIO_H__

#include "suio++.h"

class UdpSuio : public suio {

private:
  // Maximum size of message we are prepared to receive in one go
  static const ssize_t MTU_SIZE = 1600;

public:
  // Like suio::input(), except that the suio is cleared in advance,
  // and the sending address is placed in 'from' a la recvfrom. 
  int inputfrom(int sd, struct sockaddr *from, socklen_t *fromlen);
  
};

#endif /* __UDPSUIO_H__ */
