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

#include "element.h"


class PlSensor { 
  
  const char *class_name() const		{ return "Plsensor";};
  const char *processing() const		{ return "/hh"; };
  const char *flow_code() const		{ return "/-"; };

public:

  PlSensor(u_int16_t sensor_port, str sensor_path, timespec reconnect_delay);

private:
  
  // States
  enum State { ST_CONNECTING, 
	       ST_SENDING, 
	       ST_RECEIVING, 
	       ST_BLOCKED,
	       ST_WAITING };
  State		state;

  // Socket details
  u_int16_t	port;
  str		path;
  int		sd; 
  tcpconn_t	tc;

  // Time between reconnects
  timespec	delay;

  // The request string
  strbuf	request;
};


#endif /* __PLSENSOR_H_ */
