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
 * DESCRIPTION: Element for scanning a PLanetLab Sensor Interface sensor
 */

#ifndef __PLSENSOR_H__
#define __PLSENSOR_H__

#include "config.h"
#include "element.h"
#include "fdbuf.h"
#include "loop.h"

#include <boost/regex.hpp>

class PlSensor : public Element { 
  
  const char *class_name() const		{ return "PlSensor";};
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/h"; }

public:

  PlSensor(string,
           u_int16_t sensor_port, string sensor_path, uint32_t reconnect_delay);

private:

  void enter_connecting();
  void error_cleanup(uint32_t errnum, string errmsg);
  void enter_waiting();
  void connect_cb(int fd);
  void write_cb();
  void rx_hdr_cb();
  void rx_body_cb();
  void socket_on() { fileDescriptorCB(sd, b_selread,
                                      boost::bind(&PlSensor::rx_body_cb, this)); };
  void socket_off() { removeFileDescriptorCB(sd, b_selread); };
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
  string	path;
  int		sd; 
  tcpHandle*    tc;
  boost::regex	req_re;
  Fdbuf		hdrs;
  in_addr	localaddr;
  timeCBHandle  *wait_delaycb;


  // Time between reconnects
  double	delay;

  // The request string
  Fdbuf		reqtmpl;
  Fdbuf		req_buf;
};



#endif /* __PLSENSOR_H_ */
