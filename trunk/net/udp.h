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

#ifndef __UDP_H__
#define __UDP_H__

#include "element.h"

//
// The Udp::Rx element emits 2-tuples:
//   1) A string containing the received packet payload
//   2) A string consisting of a struct sockaddr representing where
//     the packet came from.   
// In addition, the element has a second push output which sends
// tuples consisting of a single integer representing the errno if
// there is a receive error. 
// 
// Udp::Tx elements are pull-input and take 2-tuples as above.  They
// also have a single push-output for errors as above. 
//

class Udp { 
public:

  // 
  // First, the Rx element: pushes tuples when packets arrive
  //
  class Rx : public Element {
  public:
    Rx(const Udp &udp);
    const char *class_name() const		{ return "Udp::Rx";};
    const char *processing() const		{ return "/hh"; };
    const char *flow_code() const		{ return "/-"; };
  private:
    void socket_cb(); // Callback for socket activity
    void element_cb(); // Callback for element push 

    void socket_on() { fdcb(u->sd, selread, wrap(this,&Udp::Rx::socket_cb)); };
    void socket_off() { fdcb(u->sd, selread, NULL); };
    
    const Udp *u;
    bool push_pending;  // == push enabled (return value of last push)
    
  };
  
  // 
  // Second, the Tx element: pulls tuples when the socket can send
  //
  class Tx : public Element {
  public:
    Tx(const Udp &udp);
    const char *class_name() const		{ return "Udp::Tx";};
    const char *processing() const		{ return "l/h"; };
    const char *flow_code() const		{ return "-/"; };
  private:
    void socket_cb(); // Callback for socket activity
    void element_cb(); // Callback for element pull

    void socket_on() { fdcb(u->sd, selwrite, wrap(this,&Udp::Tx::socket_cb));};
    void socket_off() { fdcb(u->sd, selwrite, NULL); };
    
    const Udp *u;
    bool pull_pending;  // == pull enabled (there is a pull to do)
  };
  
  //
  // Now the Udp object itself.
  //
  Udp(u_int16_t port=0, u_int32_t addr = INADDR_ANY);

  // Accessing the individual elements
  ref< Udp::Rx > get_rx() { return rx; };
  ref< Udp::Tx > get_tx() { return tx; };

private:
  // Socket
  int sd; 

  // Elements 
  ref< Rx > rx;
  ref< Tx > tx;
};

#endif /* __UDP_H_ */
