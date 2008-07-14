// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: PlanetLab sensor element
 */

#include "plsensor.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "val_str.h"

//#define TRACE_OFF
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
    "^HTTP/1.\\d+\\s+(\\d\\d\\d)\\s+(.+)\\r?\\n((.+\\r?\\n)+)\\r?\\n((.*\\n)*)"

//
// Constructor.  Following errors (or an end of file), the sensor
// element will attempt to reconnect to the server after waiting for
// reconnect_delay seconds. 
//
PlSensor::PlSensor(string name,
                   u_int16_t sensor_port, 
		   string sensor_path, 
		   uint32_t reconnect_delay)
  : Element(name, 0,1),
    port(sensor_port), 
    path(sensor_path), 
    sd(-1),
    // req_re(HTTP_RX), FIX ME
    delay(reconnect_delay)
{
  TRC_FN;
  inet_aton("127.0.0.1", &localaddr);
  reqtmpl << "GET " << path << " HTTP/1.0\r\n" 
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
void PlSensor::error_cleanup(uint32_t errnum, string errmsg) 
{
  TRC_FN;
  log(LoggerI::WARN, errnum, errmsg);
  enter_waiting();
}

//
// Clean everything up, then go to sleep for reconnect_delay seconds
// before trying again. 
//
void PlSensor::enter_waiting()
{
  TRC_FN;
  if (sd >= 0) { 
    removeFileDescriptorCB(sd, b_selread);
    removeFileDescriptorCB(sd, b_selwrite);
    close(sd);
    sd = -1;
  }
  state = ST_WAITING;
  wait_delaycb = delayCB(delay, boost::bind(&PlSensor::enter_connecting,this), this);
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
  hdrs.clear();
  req_buf.clear();
  req_buf << reqtmpl.str();
  tc = tcpConnect(localaddr, port, boost::bind(&PlSensor::connect_cb,this,_1));
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
    error_cleanup(errno, string() + "Connecting:" + strerror(errno));
  } else {
    // Enter SENDING
    TRC( "socket descriptor is " << sd);
    state = ST_SENDING;
    fileDescriptorCB(sd, b_selwrite, boost::bind(&PlSensor::write_cb,this), this);
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
  if (req_buf.length()) {
    // Still stuff to send
    size_t s = req_buf.write(sd);
    TRC( "wrote " << s);
    if ( s < 0 && errno != EAGAIN && errno !=0) {
      error_cleanup(errno, string("Writing request:") + strerror(errno));
      return;
    }
  }
  // Falls through to here when the sending is done...
  if (req_buf.length() == 0) {
    // Enter RX_HEADERS state
    removeFileDescriptorCB(sd, b_selwrite);
    state = ST_RX_HEADERS;
    fileDescriptorCB(sd, b_selread, boost::bind(&PlSensor::rx_hdr_cb,this), this);
  }
}

//
// Socket callback when we're still reading the response headers from
// the sensor server.
//
void PlSensor::rx_hdr_cb()
{
  TRC_FN;
  Fdbuf rx;
  switch (rx.read(sd)) {
  case 0:
    TRC("Got 0; Read: " << hdrs.str());
    TRC("Errno: " << errno );
    error_cleanup(errno, "Server closed connection reading headers");
    return;
  case -1:
    if (errno != EAGAIN) {
      error_cleanup(errno, string("Reading headers:") +  strerror(errno));
    }
    return;
  default:
    hdrs << rx.str();
    if (hdrs.length() > MAX_REQUEST_SIZE ) {
      error_cleanup(100000, "Headers too large");
      break;
    }
    // Now try and match the regular expression
    boost::smatch m;
    if ( regex_search( hdrs.str(),m,req_re )) {
      // Create a tuple with the single string, the matching body.
      TuplePtr t = Tuple::mk();
      t->append(Val_Str::mk(m[5]));
      if (push(0,t,boost::bind(&PlSensor::element_cb,this))) {
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

  Fdbuf rx;
  switch (rx.read(sd)) {
  case 0:
    TRC("Connection closed: enter waiting");
    // Not really an error - just retry.
    socket_off();
    enter_waiting();
    return;
  case -1:
    if (errno != EAGAIN) {
      error_cleanup(errno, string("Reading body:") + strerror(errno));
    }
    return;
  default:
    TuplePtr t = Tuple::mk();
    t->append(Val_Str::mk(rx.str()));
    if (push(0,t,boost::bind(&PlSensor::element_cb,this))) {
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
