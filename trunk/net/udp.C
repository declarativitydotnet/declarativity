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

/////////////////////////////////////////////////////////////////////
//
// Receive element
//
Udp::Rx::Rx(const Udp &udp) 
  : Element(0, 1),
    u(&udp), push_pending(true) 
{
  socket_on();
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
  UdpSuio *uio = New UdpSuio();
  struct sockaddr sa;
  socklen_t sa_len;
  if ( uio->inputfrom(u->sd, &sa, &sa_len) <= 0) {
    // Error! 
    if (errno != EAGAIN) {
      // Make an error tuple... 
      TupleRef t = Tuple::mk();
      t->append(New refcounted<TupleField>(errno));
      push(1,t,cbv_null);
    }
    delete uio;
  } else {
    // Success! We've got a packet.  Package it up...
    TupleRef t = Tuple::mk();
    t->append(New refcounted<TupleField>(str(*uio)));
    t->append(New refcounted<TupleField>(str( (const char *)&sa, sa_len)));
    // Push it. 
    push_pending = push(0,t,wrap(this,&Udp::Rx::element_cb));
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


/////////////////////////////////////////////////////////////////////
//
// Transmit element
//
Udp::Tx::Tx(const Udp &udp) 
  : u(&udp), pull_pending(true) 
{
  socket_on();
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
  TuplePtr t = input(0)->pull(wrap(this,&Udp::Tx::element_cb));
  if (!t) {
    pull_pending = false;
    socket_off();
    return;
  }
  
  // We've now go a packet...
  str payload = (*t)[0]->as_s();
  str addr = (*t)[1]->as_s();
  ssize_t s = sendto(u->sd, 
		     payload.cstr(), payload.len(),
		     0, 
		     (struct sockaddr *)(addr.cstr()), addr.len());
  // 's' is signed, whereas the payload.len() isn't. Hence the following:
  if (s <= 0 || (unsigned)s <= payload.len() ) {
    // Error!  Technically, this can happen if the payload is larger
    //  than the socket buffer (in which case errno=EAGAIN).  We treat
    //  this as an error, nevertheless, and leave it up to the
    //  segmentation and reassembly elements upstream to not make us
    //  send anything bigger than the MTU, which should fit into the
    //  socket buffers. 
    // Make an error tuple... 
    TupleRef t = Tuple::mk();
    t->append(New refcounted<TupleField>(errno));
    push(1,t,cbv_null);
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

////////////////////////////////////////////////////////////////////
//
// The main object itself
//
Udp::Udp(u_int16_t port, u_int32_t addr) 
  : rx(New refcounted< Udp::Rx >(*this)),
    tx(New refcounted< Udp::Tx >(*this))
{
  sd = inetsocket(SOCK_DGRAM, port, addr);
  make_async(sd);
}
