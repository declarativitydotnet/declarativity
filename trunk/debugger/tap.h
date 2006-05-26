/*
 * @(#)$Id$
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
