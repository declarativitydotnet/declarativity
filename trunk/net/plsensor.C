// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * Copyright (c) 2004 Intel Corporation
 * All rights reserved.
 *
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: PlanetLab sensor element
 */

#include "plsensor.h"
#include <async.h>
#include <sys/types.h>
#include <sys/socket.h>

#define TRACE_OFF
#include "trace.h"

//
// This regexp matches an HTTP response.  Various groups are of
// interest:
//  1: The numeric return code
//  2: The explanation of (1)
//  3: The complete set of headers, followed by the blank line.
//  5: The first part of the document body (possibly all of it)
//
#define HTTP_RX \
    "^HTTP/1.\\d+\\s+(\\d\\d\\d)\\s+(.+)\\r\\n((.+\\r\\n)+)\\r\\n((.*\\n)*)"

//
// Constructor.  Following errors (or an end of file), the sensor
// element will attempt to reconnect to the server after waiting for
// reconnect_delay seconds. 
//
PlSensor::PlSensor(u_int16_t sensor_port, 
		   str sensor_path, 
		   uint32_t reconnect_delay )
  : Element(0,1),
    port(sensor_port), 
    path(sensor_path), 
    sd(-1),
    delay(reconnect_delay),
    req_re(HTTP_RX),
    req_buf(NULL)
{
  TRC_FN;
  inet_aton("127.0.0.1", &localaddr);
  reqtmpl = strbuf() << "GET " << path << " HTTP/1.0\r\n" 
	  << "Host: localhost:" << port << "\r\n"
	  << "Connection: close\r\n"
	  << "User-Agent: P2SensorScanner/0.1 (Intel Berkeley P2 dataflow engine)\r\n"
	  << "\r\n";
  enter_connecting();
}

//
// Called from whenever there is an error.  More work neede here,
// clearly!  When we have a clean way to access the logging element,
// log the tuple and then enter_waiting, below. 
// 
void PlSensor::error_cleanup(uint32_t errnum, str errmsg) 
{
  TRC_FN;
  warn << "PlSensor " << path << ":" << port << " " << errnum << " " << errmsg << "\n";
  // Generate an error tuple.
  exit(1);
}

//
// Clean everything up, then go to sleep for reconnect_delay seconds
// before trying again. 
//
void PlSensor::enter_waiting()
{
  TRC_FN;
  if (sd >= 0) { 
    close(sd);
    sd = -1;
  }
  if (req_buf != NULL) {
    delete req_buf;
    req_buf = NULL;
  }
  if (hdrs != NULL) {
    delete hdrs;
    hdrs = NULL;
  }
  state = ST_WAITING;
  wait_delaycb = delaycb( delay, wrap(this, &PlSensor::enter_connecting));
}

//
// Enter connecting state.  This is either entered from
// initialization, or from a delayed callback installed by
// enter_waiting above. 
//
void PlSensor::enter_connecting() 
{
  TRC_FN;
  state = ST_CONNECTING;
  assert(sd < 0);
  assert(req_buf == NULL);
  assert(hdrs == NULL);
  hdrs = New strbuf();
  req_buf = New strbuf(reqtmpl);
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
    TRC( "socket descriptor is " << sd);
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
  assert(req_buf != NULL);
  if (req_buf->tosuio()->resid()) {
    // Still stuff to send
    size_t s = req_buf->tosuio()->output(sd);
    TRC( "wrote " << s);
    if ( s < 0 && errno != EAGAIN && errno !=0) {
      error_cleanup(errno, strbuf() << "Writing request:" << strerror(errno));
      return;
    }
  }
  // Falls through to here when the sending is done...
  if (req_buf->tosuio()->resid() == 0) {
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
    error_cleanup(errno, strbuf() << "Server closed connection reading headers:" << strerror(errno));
    return;
  case -1:
    if (errno != EAGAIN) {
      error_cleanup(errno, strbuf() << "Reading headers:" << strerror(errno));
    }
    return;
  default:
    *hdrs << rx;
    if (hdrs->tosuio()->resid() > MAX_REQUEST_SIZE ) {
      error_cleanup(100000, "Headers too large");
      break;
    }
    // Now try and match the regular expression
    rxx::matchresult m = req_re.search(str(*hdrs));
    TRC("Success: " << req_re.success());
    if (m) {
      // Create a tuple with the single string, the matching body.
      TupleRef t = Tuple::mk();
      t->append(New refcounted<TupleField>(m[5]));
      if (push(0,t,wrap(this,&PlSensor::element_cb))) {
	socket_on();
	state = ST_RX_BODY;
      } else {
	socket_off();
	state = ST_BLOCKED;
      }
    } else {
      TRC("No match so far.");
    }
  }
}

//
// Why you might think this might not work:
// 
//  Keeping track of whether pushes are allowed or not doesn't seem to
//  match up with the reconnection logic, such that one can have a
//  situation where the element reconnects to the sensor server, and
//  pushes the first part of the body (in rx_hdr_cb above) when pushes
//  are in fact disabled.  However, I (Mothy) don't *think* this can
//  happen, since rx_body_cb() will not be called if pushes are
//  blocked.  Consequently, the system will never enter ST_CONNECTING
//  unless pushes are enabled.  Seem plausible?

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
    TRC("Connection closed: enter waiting");
    // Not really an error - just retry.
    socket_off();
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
    if (push(0,t,wrap(this,&PlSensor::element_cb))) {
      socket_on();
      state = ST_RX_BODY;
    } else {
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
