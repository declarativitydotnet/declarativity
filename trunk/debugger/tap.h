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
 * DESCRIPTION: It has one input and two output
 * ports, one connecting to the rule strand and other 
 * connecting to the ruleTracer element.
 * Whatever tuple it recieves on the input, 
 * it passes a copy of the tuple to the ruleTracer 
 * element. It will never block on the ruleTracer
 * since ruleTracer always accepts. 
 *
 */

#ifndef __TAP_H__
#define __TAP_H__

#include "element.h"

class Tap : public Element {
 public:

  Tap(string ruleName, int ruleNum);
  
  ~Tap();

  /** Overridden to tap flowing tuples
   **/
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "Tap";}
  const char *processing() const		{ return "a/a";}
  const char *flow_code() const			{ return "-/-";}

 private:
  int _ruleNum;

};

#endif /* __TAP_H__ */
