// -*- c-basic-offset: 2; related-file-name: "slot.C" -*-
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
 * DESCRIPTION: an element that blocks/unblocks on signal
 *   		AND AUTOMAATICALLY turns off when a tuple
 *		has passed 
 */

#ifndef __ONE_OFF_SWITCH_H__
#define __ONE_OFF_SWITCH_H__

#include "switch.h"

class OneOffSwitch : public Switch { 
public:
  
  OneOffSwitch(string name);

  const char *class_name() const		{ return "OneOffSwitch";}
  const char *processing() const		{ return "l/h"; }
  const char *flow_code() const			{ return "-/-"; }


  OneOffSwitch(TuplePtr);

  void run(); 

};


#endif /* __BLOCKER_H_ */
