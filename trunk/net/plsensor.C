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

#include "plsensor.h"

#include <async.h>
#include <sys/types.h>
#include <sys/socket.h>

PlSensor::PlSensor(u_int16_t sensor_port, str sensor_path, timespec reconnect_delay)
  : port(sensor_port), path(sensor_path), delay(reconnect_delay)
{
  request << "GET " << path << " HTTP/1.0\r\n" 
	  << "Host: localhost:" << port << "\r\n"
	  << "Connection: close\r\n"
	  << "User-Agent: P2 Sensor Scanner\r\n"
	  << "\r\n";
  enter_connecting();
}

void PlSensor::enter_connecting() 
{
  state = ST_CONNECTING;
  tc = tcpconnect( localaddr, port, wrap(this, &PlSensor::connect_cb));
}

void PlSensor::connect_cb(int fd) 
{
  sd = fd;
  enter_sending();
}

void PlSensor::enter_sending()
{
  state = ST_SENDING;
  fdcb(sd, selwrite, wrap(this, &PlSensor::write_cb));
  // And set up request string...
}

void PlSensor::write_cb()
{
  if (request.tosuio()->resid()) {
    // Still stuff to send
    if ( request.tosuio()->output(fd) && errno != EAGAIN && errno !=0) {
      DBG("header output error: " << strerror(errno));
      cleanup();
    } else {
      TRC("headers left: " << headers.tosuio()->resid());
    }
    return;
  } else {
    // No more of the request to send..
    fdcb(sd, selwrite, NULL);
    enter_receiving()
  }
}

void PlSensor::enter_receiving()
{
  state = ST_RECEIVING;
  fdcb(sd, selread, wrap(this, &PlSensor::read_cb));
}

void PlSensor::read_cb()
{
  


  strbuf rx;
  switch (rx.tosuio()->input(fd)) {
  case 0:
    DBG("Client closed connection before headers");
    enter_waiting();
    break;
  case -1:
    if (errno != EAGAIN) {
      DBG("Read error: " << strerror(errno));
      enter_waiting();
    }
    break;
  default:
    TupleRef t = Tuple::mk();
    TupleFieldRef tv = New recounted<TupleField>(str(buf));
    push_pending = push(0,t,wrap(this,&PlSensor::push_cb));
    

      fdcb(fd, selread, NULL );
      process_req(s);
    } else if (s.len() > MAX_REQUEST_SIZE ) {
      DBG("HUGE request"); 
      snap_response( "400 Waay too large!", NULL);
      fdcb(fd, selread, NULL );
    }
    

  
