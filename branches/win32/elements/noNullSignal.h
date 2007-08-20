/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element that only passes through non-empty tuples.
 *              Upon an empty tuple will push empty tuple to output port 1.
 */

#ifndef __NONULL_SIGNAL_H__
#define __NONULL_SIGNAL_H__

#include "element.h"
#include "elementRegistry.h"

class NoNullSignal : public Element { 
public:
  NoNullSignal(string);
  NoNullSignal(TuplePtr args);
  ~NoNullSignal();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const { return "NoNullSignal";}
  const char *processing() const { return "a/ah"; }
  const char *flow_code() const	 { return "x/x-"; }

  DECLARE_PUBLIC_ELEMENT_INITS

private:

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __NONULL_SIGNAL_H_ */
