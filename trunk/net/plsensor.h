// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element for scanning a PLanetLab Sensor Interface sensor
 */

#ifndef __PLSENSOR_H__
#define __PLSENSOR_H__

#include "config.h"
#include "element.h"
#include <async.h>
#include <rxx.h>

class PlSensor : public Element { 
  
  const char *class_name() const		{ return "PlSensor";};
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/h"; }

public:

  PlSensor(str,
           u_int16_t sensor_port, str sensor_path, uint32_t reconnect_delay);

private:

  //
  // Different versions of libasync have different names for a TCP
  // connection-in-progress handle...
  //
#ifdef HAVE_TCPCONN_T_P
  typedef tcpconn_t conn_t;
#else
#ifdef HAVE_TCPCONNECT_T_P
  typedef tcpconnect_t *conn_t;
#else
#error No TCP connection type from libasync!
#endif
#endif

  void enter_connecting();
  void error_cleanup(uint32_t errnum, str errmsg);
  void enter_waiting();
  void connect_cb(int fd);
  void write_cb();
  void rx_hdr_cb();
  void rx_body_cb();
  void socket_on() { fdcb(sd, selread, wrap(this,&PlSensor::rx_body_cb)); };
  void socket_off() { fdcb(sd, selread, NULL); };
  void element_cb();
  
  static const size_t MAX_REQUEST_SIZE = 10000;
  
  // States
  enum State { ST_CONNECTING, 
	       ST_SENDING, 
	       ST_RX_HEADERS,
	       ST_RX_BODY, 
	       ST_BLOCKED,
	       ST_WAITING };
  State		state;

  // Socket details
  u_int16_t	port;
  str		path;
  int		sd; 
  conn_t        tc;
  rxx		req_re;
  strbuf       *hdrs;
  in_addr	localaddr;
  timeCBHandle *wait_delaycb;


  // Time between reconnects
  uint32_t	delay;

  // The request string
  str		reqtmpl;
  strbuf       *req_buf;
};



#endif /* __PLSENSOR_H_ */
