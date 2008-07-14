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
 * DESCRIPTION: Element-pair for a UDP socket
 */

#include "udp2.h"
#include "tuple.h"
#include <sys/types.h>
#include <sys/socket.h>

#include "val_str.h"
#include "val_opaque.h"
#include "val_int32.h"

/////////////////////////////////////////////////////////////////////
//
// Receive element
//
Udp2::Rx::Rx(string name, Udp2 &udp) 
    : u(&udp), push_pending(true) 
{
}

//
// Socket callback for receive element
//
void Udp2::Rx::socket_cb()
{
  // If push is not enabled, turn off the callback and return. 
  if (!push_pending) {
    socket_off();
    return;
  }

  // Read packet. 
  FdbufPtr fb(new Fdbuf());
  struct sockaddr sa;
  bzero(&sa, sizeof(sa));
  socklen_t sa_len = 0;
  int result = fb->recvfrom(u->sd, Fdbuf::BUF_DFLT_READ, 0, &sa, &sa_len);
  if (result <= 0) {
    // Error! 
    int error = errno;
    if (error != EAGAIN) {
      u->log(LoggerI::ERROR, error, strerror(error));
    }
  } else {
    // Success! We've got a packet.  Package it up...
    FdbufPtr addressFb(new Fdbuf());
    addressFb->pop_bytes(reinterpret_cast<char *>(&sa), sa_len);

    TuplePtr t = Tuple::mk();
    t->append(Val_Opaque::mk(addressFb));
    t->append(Val_Opaque::mk(fb));
    t->freeze();
    // Push it. 
    push_pending = u->push(0, t, boost::bind(&Udp2::Rx::element_cb,this));
  }
  socket_on();
}

//
// Element callback: called when push is re-enabled. 
//
void Udp2::Rx::element_cb()
{
  push_pending = true;
  socket_on();
}

int Udp2::Rx::initialize()
{
  socket_on();
  return 0;
}


/////////////////////////////////////////////////////////////////////
//
// Transmit element
//
Udp2::Tx::Tx(string name, Udp2 &udp) 
    : u(&udp), pull_pending(true)
{
}

//
// Socket callback for transmit element
//
void Udp2::Tx::socket_cb()
{
  // If pull is not enabled, turn off the callback and return. 
  if (!pull_pending) {
    socket_off();
    return;
  }

  // Try to pull a packet. 
  Element::PortPtr myInput = u->input(0);
  TuplePtr t = myInput->pull(boost::bind(&Udp2::Tx::element_cb,this));
  if (!t) {
    pull_pending = false;
    socket_off();
    return;
  } 
  
  // We've now got a packet...
  struct sockaddr address;
  FdbufPtr fba = Val_Opaque::cast((*t)[0]);
  fba->pop_bytes(reinterpret_cast<char *>(&address), sizeof(address));

  FdbufPtr fbp = Val_Opaque::cast((*t)[1]);
  ssize_t s = fbp->sendto(u->sd, fbp->length(), 0, &address, sizeof(address));
  // 's' is signed, whereas the payload.len() isn't. Hence the following:
  if (s <= 0 || (size_t)s < fbp->length() ) {
    // Error!  Technically, this can happen if the payload is larger
    //  than the socket buffer (in which case errno=EAGAIN).  We treat
    //  this as an error, nevertheless, and leave it up to the
    //  segmentation and reassembly elements upstream to not make us
    //  send anything bigger than the MTU, which should fit into the
    //  socket buffers. 
    u->log(LoggerI::ERROR, errno, "Payload larger than socket buffer");
  }
  socket_on();
}

//
// Element callback: called when push is re-enabled. 
//
void Udp2::Tx::element_cb()
{
  pull_pending = true;
  socket_on();
}

int Udp2::Tx::initialize()
{
  socket_on();
  return 0;
}

////////////////////////////////////////////////////////////////////
//
// The main object itself
//
Udp2::Udp2(string name,
         u_int16_t port, u_int32_t addr) 
  : Element(name, 1, 1),
    rx(new Udp2::Rx(name, *this)),
    tx(new Udp2::Tx(name, *this))
{
  sd = networkSocket(SOCK_DGRAM, port, addr);
}
