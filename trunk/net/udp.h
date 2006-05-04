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

#ifndef __UDP_H__
#define __UDP_H__

#include "element.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "loop.h"

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
    Rx(string, const Udp &udp);
    const char *class_name() const		{ return "Udp::Rx";};
    const char *processing() const		{ return "/h"; };
    const char *flow_code() const		{ return "/-"; };

    void socket_on()
    {
      fileDescriptorCB(u->sd, b_selread,
                       boost::bind(&Udp::Rx::socket_cb, this), this);
    };
    void socket_off()
    {
      removeFileDescriptorCB(u->sd, b_selread);
    };

    /** Turn on the socket and start listening. */
    virtual int initialize();

  private:
    void socket_cb(); // Callback for socket activity
    void element_cb(); // Callback for element push 
    
    const Udp *u;
    bool push_pending;  // == push enabled (return value of last push)
    
  };
  
  // 
  // Second, the Tx element: pulls tuples when the socket can send
  //
  class Tx : public Element {
  public:
    Tx(string, const Udp &udp);
    const char *class_name() const		{ return "Udp::Tx";};
    const char *processing() const		{ return "l/"; };
    const char *flow_code() const		{ return "-/"; };

    void socket_on() { fileDescriptorCB(u->sd, b_selwrite, boost::bind(&Udp::Tx::socket_cb, this));};
    void socket_off() { removeFileDescriptorCB(u->sd, b_selwrite); };

    /** Turn on the socket */
    virtual int initialize();

  private:
    void socket_cb(); // Callback for socket activity
    void element_cb(); // Callback for element pull
    
    const Udp *u;
    bool pull_pending;  // == pull enabled (there is a pull to do)
  };
  
  //
  // Now the Udp object itself.
  //
  Udp(string, u_int16_t port=0, u_int32_t addr = INADDR_ANY);

  string toConfString() const { 
    ostringstream oss;
    oss << "Udp(\""<<_name<<"\", "<<_port<<", "<<_addr<<")"; 
    return oss.str();
  }

  // Accessing the individual elements
  boost::shared_ptr< Udp::Rx > get_rx() { return rx; };
  boost::shared_ptr< Udp::Tx > get_tx() { return tx; };

private:
  // Socket
  int sd; 

  // My name
  string _name;

  u_int16_t _port;
  u_int32_t _addr;

  // Elements 
  boost::shared_ptr< Rx > rx;
  boost::shared_ptr< Tx > tx;
};

#endif /* __UDP_H_ */
