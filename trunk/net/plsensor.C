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
#include "trace.h"
#include <async.h>
#include <sys/types.h>
#include <sys/socket.h>

PlSensor::PlSensor(u_int16_t sensor_port, 
		   str sensor_path, 
		   timespec reconnect_delay )
  : port(sensor_port), 
    path(sensor_path), 
    sd(-1),
    delay(reconnect_delay),
    req_re("HTTP/1.\\d+\\s+(\\d\\d\\d)\\s+(.+)\\r\\n((.+\\r\\n)+)\\r\\n((.*\\r\\n)+)")
{
  TRC_FN;
  request << "GET " << path << " HTTP/1.0\r\n" 
	  << "Host: localhost:" << port << "\r\n"
	  << "Connection: close\r\n"
	  << "User-Agent: P2SensorScanner/0.1 (Intel Berkeley P2 dataflow engine)\r\n"
	  << "\r\n";
  inet_aton("127.0.0.1", &localaddr);
  enter_connecting();
}


void PlSensor::error_cleanup(uint32_t errnum, str errmsg) 
{
  TRC_FN;
  warn << "PlSensor " << path << ":" << port << " " << errnum << " " << errmsg << "\n";
  // Generate an error tuple.
  exit(1);
}

void PlSensor::enter_waiting()
{
  TRC_FN;
  if (sd >= 0) { 
    close(sd);
    sd = -1;
  }
  state = ST_WAITING;
  wait_timecb = timecb( delay, wrap(this, &PlSensor::wait_cb));
}

void PlSensor::wait_cb()
{
  TRC_FN;
  assert(state == ST_WAITING);
  if (wait_timecb) {
    timecb_remove(wait_timecb);
    wait_timecb = NULL;
  }
  enter_connecting();
}

void PlSensor::enter_connecting() 
{
  TRC_FN;
  state = ST_CONNECTING;
  assert(sd < 0);
  hdrs = "";
  tc = tcpconnect( localaddr, port, wrap(this, &PlSensor::connect_cb));
}

//
// Callback when the TCP connection to the sensor server has been
// established (or something's gone wrong). 
//
void PlSensor::connect_cb(int fd) 
{
  TRC_FN;
  sd = fd;
  if (sd < 0) {
    error_cleanup(errno, strbuf() << "Connecting:" << strerror(errno));
  } else {
    // Enter SENDING
    state = ST_SENDING;
    fdcb(sd, selwrite, wrap(this, &PlSensor::write_cb));
  }
}


//
// Socket callback when we're still writing the request to the sensor
// server. 
//
void PlSensor::write_cb()
{
  TRC_FN;
  assert(state == ST_SENDING);
  if (request.tosuio()->resid()) {
    // Still stuff to send
    if ( request.tosuio()->output(sd) && errno != EAGAIN && errno !=0) {
      error_cleanup(errno, strbuf() << "Writing request:" << strerror(errno));
      return;
    }
  }
  // Falls through to here when the sending is done...
  if (request.tosuio()->resid() == 0) {
    // Enter RX_HEADERS state
    fdcb(sd, selwrite, NULL);
    state = ST_RX_HEADERS;
    fdcb(sd, selread, wrap(this, &PlSensor::rx_hdr_cb));
  }
}

//
// Socket callback when we're still reading the response headers from
// the sensor server.
//
void PlSensor::rx_hdr_cb()
{
  TRC_FN;
  strbuf rx;
  switch (rx.tosuio()->input(sd)) {
  case 0:
    // ERROR: server unexpectedly closed connection during headers 
    error_cleanup(errno, strbuf() << "Server closed connection reading headers:" << strerror(errno));
    return;
  case -1:
    if (errno != EAGAIN) {
      error_cleanup(errno, strbuf() << "Reading headers:" << strerror(errno));
    }
    return;
  default:
    hdrs << str(rx);
    if (hdrs.len() > MAX_REQUEST_SIZE ) {
      error_cleanup(100000, "Headers too large");
      break;
    }
    // Now try and match the regular expression
    rxx::matchresult m = req_re.match(hdrs);
    if (m) {
      for(int i=0; i < 6; i++ ) {
	warn << "Match:" << m[i] << "\n";
      }
      // Create a tuple with the single string, the matching body.
      state = ST_RX_BODY;
    }
  }
}

//
// Why you might think this might not work:
// 
//  - Keeping track of whether pushes are allowed or not doesn't seem
//    to match up with the reconnection logic, such that one can have
//    a situation where the element reconnects to the sensor server,
//    and pushes the first part of the body (in rx_hdr_cb above) when
//    pushes are in fact disabled.  I don't *think* this can happen,
//    since rx_body_cb() will not be called if pushes are blocked.
//    Consequently, the system will never enter ST_CONNECTING unless
//    pushes are enabled.  Seem plausible?

//
// Socket callback when we're through the headers. 
//
void PlSensor::rx_body_cb()
{
  TRC_FN;
  // If push is not enabled, turn off the callback and return. 
  if ( state != ST_RX_BODY ) {
    socket_off();
    return;
  }

  strbuf rx;
  switch (rx.tosuio()->input(sd)) {
  case 0:
    // Not really an error - just retry.
    enter_waiting();
    return;
  case -1:
    if (errno != EAGAIN) {
      error_cleanup(errno, strbuf() << "Reading body:" << strerror(errno));
    }
    return;
  default:
    TupleRef t = Tuple::mk();
    t->append(New refcounted<TupleField>(rx));
    bool push_pending = push(0,t,wrap(this,&PlSensor::element_cb));
    if (push_pending) {
      socket_off();
      state = ST_BLOCKED;
    }
  }
}

//
// The push callback
//
void PlSensor::element_cb()
{
  assert(state == ST_BLOCKED);
  state = ST_RX_BODY;
  socket_on();
}
