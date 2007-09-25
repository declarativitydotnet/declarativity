// -*- c-basic-offset: 2; related-file-name: "timedPullPush.C" -*-
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
 * DESCRIPTION: Element that at turns pull into push
 *     It will push whenever there is something to pull
 *     and can be pushed downstream 
 */


#ifndef __PULL_PUSH_H__
#define __PULL_PUSH_H__

#include "element.h"
#include "elementRegistry.h"

class PullPush : public Element { 
public:
  
  PullPush(string name);
  PullPush(TuplePtr);
  
  const char *class_name() const		{ return "PullPush"; }
  const char *flow_code() const			{ return "-/-"; }
  const char *processing() const		{ return "l/h"; }
  
  /*overriding element code*/
  int initialize();
  
  DECLARE_PUBLIC_ELEMENT_INITS
  
private:
  
  /** My pull wakeup callback */
  b_cbv pull_cb;
  void  pull_fn();
  bool  pull_ready;
  
  /** My push wakeup callback */
  b_cbv push_cb;
  void  push_fn();
  bool  push_ready;
  
  void  run();

  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __PULL_PUSH_H_ */
