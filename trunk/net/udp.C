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

#include "udp.h"
#include "udpsuio.h"
#include "tuple.h"
#include <async.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "val_str.h"
#include "val_opaque.h"
#include "val_int32.h"

/////////////////////////////////////////////////////////////////////
//
// Receive element
//
Udp::Rx::Rx(str name, const Udp &udp) 
  : Element(name, 0, 1),
    u(&udp),
    push_pending(true) 
{
}

//
// Socket callback for receive element
//
void Udp::Rx::socket_cb()
{
  // If push is not enabled, turn off the callback and return. 
  if (!push_pending) {
    socket_off();
    return;
  }

  // Read packet. 
  ref< UdpSuio > udpSuio = New refcounted< UdpSuio >;
  struct sockaddr sa;
  bzero(&sa, sizeof(sa));
  socklen_t sa_len = 0;
  if ( udpSuio->inputfrom(u->sd, &sa, &sa_len) <= 0) {
    // Error! 
    int error = errno;
    if (error != EAGAIN) {
      log(LoggerI::ERROR, error, strerror(error));
    }
  } else {
    // Success! We've got a packet.  Package it up...
    ref< suio > addressUio = New refcounted< suio >;
    addressUio->copy(&sa, sa_len);

    TuplePtr t = Tuple::mk();
    t->append(Val_Opaque::mk(addressUio));
    t->append(Val_Opaque::mk(udpSuio));
    t->freeze();
    // Push it. 
    push_pending = push(0, t, boost::bind(&Udp::Rx::element_cb, this));
  }
  socket_on();
}

//
// Element callback: called when push is re-enabled. 
//
void Udp::Rx::element_cb()
{
  push_pending = true;
  socket_on();
}

int Udp::Rx::initialize()
{
  socket_on();
  return 0;
}


/////////////////////////////////////////////////////////////////////
//
// Transmit element
//
Udp::Tx::Tx(str name, const Udp &udp) 
  : Element(name, 1, 0),
    u(&udp),
    pull_pending(true)
{
}

//
// Socket callback for receive element
//
void Udp::Tx::socket_cb()
{
  // If pull is not enabled, turn off the callback and return. 
  if (!pull_pending) {
    socket_off();
    return;
  }

  // Try to pull a packet. 
  Element::PortPtr myInput = input(0);
  TuplePtr t = myInput->pull(boost::bind(&Udp::Tx::element_cb, this));
  if (!t) {
    pull_pending = false;
    socket_off();
    return;
  }
  
  // We've now got a packet...
  struct sockaddr address;
  ref< suio > uio = Val_Opaque::cast((*t)[0]);
  uio->copyout(&address, sizeof(address));

  ref< suio > puio = Val_Opaque::cast((*t)[1]);
  ssize_t payloadLength = puio->resid();
  char* payloadBuffer[payloadLength];
  puio->copyout(&payloadBuffer, payloadLength);

  ssize_t s = sendto(u->sd, 
		     &payloadBuffer, payloadLength,
		     0, 
		     &address, sizeof(address));
  // 's' is signed, whereas the payload.len() isn't. Hence the following:
  if (s <= 0 || s < payloadLength ) {
    // Error!  Technically, this can happen if the payload is larger
    //  than the socket buffer (in which case errno=EAGAIN).  We treat
    //  this as an error, nevertheless, and leave it up to the
    //  segmentation and reassembly elements upstream to not make us
    //  send anything bigger than the MTU, which should fit into the
    //  socket buffers. 
    log(LoggerI::ERROR, errno, "Payload larger than socket buffer");
  }
  socket_on();
}

//
// Element callback: called when push is re-enabled. 
//
void Udp::Tx::element_cb()
{
  pull_pending = true;
  socket_on();
}

int Udp::Tx::initialize()
{
  socket_on();
  return 0;
}

////////////////////////////////////////////////////////////////////
//
// The main object itself
//
Udp::Udp(str name,
         u_int16_t port, u_int32_t addr) 
  : _name(name),
    rx(new Udp::Rx(_name, *this)),
    tx(new Udp::Tx(_name, *this))
{
  sd = inetsocket(SOCK_DGRAM, port, addr);
  make_async(sd);
  close_on_exec(sd);
}
