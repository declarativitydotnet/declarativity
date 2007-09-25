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

#include "udp.h"
#include "tuple.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <iostream>

#include "val_str.h"
#include "val_opaque.h"
#include "val_int64.h"


DEFINE_ELEMENT_INITS(Udp, "Udp")

/////////////////////////////////////////////////////////////////////
//
// Receive element
//
Udp::Rx::Rx(string name, const Udp &udp) 
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
  FdbufPtr fb(new Fdbuf());
  struct sockaddr sa;
  bzero(&sa, sizeof(sa));
  socklen_t sa_len = 0;
  int result = fb->recvfrom(u->sd, Fdbuf::BUF_DFLT_READ, 0, &sa, &sa_len);
  if (result <= 0) {
    // Error! 
    int error = errno;
    if (error != EAGAIN) {
      ELEM_ERROR(strerror(error));
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
    push_pending = push(0, t, boost::bind(&Udp::Rx::element_cb,this));
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
Udp::Tx::Tx(string name, const Udp &udp) 
  : Element(name, 1, 0),
    u(&udp),
    pull_pending(true)
{
}

//
// Socket callback for transmit element
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
  TuplePtr t = myInput->pull(boost::bind(&Udp::Tx::element_cb,this));
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
  if (s <= 0 || (uint32_t) s < fbp->length() ) {
    // Error!  Technically, this can happen if the payload is larger
    //  than the socket buffer (in which case errno=EAGAIN).  We treat
    //  this as an error, nevertheless, and leave it up to the
    //  segmentation and reassembly elements upstream to not make us
    //  send anything bigger than the MTU, which should fit into the
    //  socket buffers. 
    ELEM_ERROR("Payload larger than socket buffer");
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
Udp::Udp(string name,
         u_int16_t port,
         u_int32_t addr) 
  : Element(name, 0, 0),
    _name(name),
    _port(port),
    _addr(addr),
    rx(new Udp::Rx(_name + "Rx", *this)),
    tx(new Udp::Tx(_name + "Tx", *this))
{
  sd = P2Net::inetSocket(SOCK_DGRAM, port, addr);
  if (sd < 0) {
    // Couldn't allocate network socket
    throw NetworkException("Couldn't allocate network socket");
  }
}


Udp::Udp(TuplePtr args) 
  : Element(Val_Str::cast((*args)[2]), 0, 0),
    rx(new Udp::Rx(name(), *this)),
    tx(new Udp::Tx(name(), *this))
{
  if (args->size() > 4) {
    sd = P2Net::inetSocket(SOCK_DGRAM, 
			   Val_Int64::cast((*args)[3]), 
			   Val_Int64::cast((*args)[4]));
  }
  else {
    sd = P2Net::inetSocket(SOCK_DGRAM, 
			   (in_addr_t)Val_Int64::cast((*args)[3]), INADDR_ANY); 
  }

  if (sd < 0) {
    // Couldn't allocate network socket
    throw NetworkException("Couldn't allocate network socket.");
  }
}


void
Udp::Rx::socket_on()
{
  EventLoop::loop()->add_read_fcb(u->sd, boost::bind(&Udp::Rx::socket_cb, this));
}

void
Udp::Rx::socket_off()
{
  EventLoop::loop()->del_read_fcb(u->sd);
}

Udp::NetworkException::NetworkException(string msg)
  : Element::Exception(msg)
{
}
