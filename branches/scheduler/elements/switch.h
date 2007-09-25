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
 * DESCRIPTION: an element that blocks/unblocks on signal or on
 * passing the specified number of tuples
 */

#ifndef __SWITCH_H__
#define __SWITCH_H__

#include "element.h"
#include "elementRegistry.h"

class Switch : public Element { 
public:
  
  /** nTuple == 0 means infinite amount of tuples, with no switch off. */
  Switch(string name, int nTuple = 0);
  Switch(TuplePtr);

  const char *class_name() const { return "Switch";}
  const char *processing() const { return "l/h"; }
  const char *flow_code() const	 { return "-/-"; }
  
  /*overriding element code*/
  int initialize();

  /* Turn the switch off and on */
  void set_state(bool torun);

  DECLARE_PUBLIC_ELEMENT_INITS

protected:
  
  /** My pull wakeup callback */
  b_cbv pull_cb;
  void  pull_fn();
  bool  pull_ready;
  
  /** My push wakeup callback */
  b_cbv push_cb;
  void  push_fn();
  bool  push_ready;

  /** Turned on or off */
  bool  running;
  b_cbv run_cb;

  /** Number of tuples to push */
  int mNTuple;

  void run();

private:
  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __BLOCKER_H_ */
