// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element-pair for a UDP socket
 */

#ifndef __UDP2_H__
#define __UDP2_H__

#include "element.h"
#include "elementRegistry.h"
#include "eventLoop.h"
#include "p2net.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>

//
// The Udp2::Rx element emits 2-tuples:
//   1) A string containing the received packet payload
//   2) A string consisting of a struct sockaddr representing where
//     the packet came from.   
// In addition, the element has a second push output which sends
// tuples consisting of a single integer representing the errno if
// there is a receive error. 
// 
// Udp2::Tx elements are pull-input and take 2-tuples as above.  They
// also have a single push-output for errors as above. 
//

class Udp2 : public Element { 
public:

  /** A network exception for the UDP element */
  class NetworkException {
  };


  // 
  // First, the Rx element: pushes tuples when packets arrive
  //
  class Rx : public Element{
  public:
    Rx(string, Udp2 &udp);

    virtual ~Rx() {};

    
    const char*
    class_name() const;

    void socket_on()
    {
      EventLoop::loop()->add_read_fcb(u->sd, 
				      boost::bind(&Udp2::Rx::socket_cb, this));
    };
    void socket_off()
    {
      EventLoop::loop()->del_read_fcb(u->sd);
    };

    /** Turn on the socket and start listening. */
    virtual int initialize();

  private:
    void socket_cb(); // Callback for socket activity
    void element_cb(); // Callback for element push 
    
    Udp2 *u;
    bool push_pending;  // == push enabled (return value of last push)
    
  };
  
  // 
  // Second, the Tx element: pulls tuples when the socket can send
  //
  class Tx : public Element {
  public:
    Tx(string, Udp2 &udp);

    const char*
    class_name() const;

    virtual ~Tx() {};

    void socket_on() { 
      EventLoop::loop()->add_write_fcb(u->sd, 
				       boost::bind(&Udp2::Tx::socket_cb,this));
    };
    void socket_off() { 
      EventLoop::loop()->del_write_fcb(u->sd);
    };

    /** Turn on the socket */
    virtual int initialize();

  private:
    void socket_cb(); // Callback for socket activity
    void element_cb(); // Callback for element pull
    
    Udp2 *u;
    bool pull_pending;  // == pull enabled (there is a pull to do)
  };
  
  //
  // Now the Udp2 object itself.
  //
  Udp2(string, u_int16_t port=0, u_int32_t addr = INADDR_ANY);
  Udp2(TuplePtr args);

  const char *class_name() const	{ return "Udp2";};
  const char *processing() const	{ return "l/h"; };
  const char *flow_code() const		{ return "-/-"; };

  virtual int initialize() {
     tx->initialize();
     rx->initialize();
     return 0;
  };

  // Accessing the individual elements
  boost::shared_ptr< Udp2::Rx > get_rx() { return rx; };
  boost::shared_ptr< Udp2::Tx > get_tx() { return tx; };

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  // Socket
  int sd; 

  // My name
  string    _name;

  // Elements 
  boost::shared_ptr< Rx > rx;
  boost::shared_ptr< Tx > tx;

  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __UDP2_H_ */
