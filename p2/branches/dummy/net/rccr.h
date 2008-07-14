/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#ifndef __RCCR_H__
#define __RCCR_H__

#include <deque>
#include <vector>
#include "tuple.h"
#include "element.h"
#include "elementRegistry.h"
#include "inlines.h"

class RateCCR : public Element {
public:
  RateCCR(string name);
  RateCCR(TuplePtr args);
  const char *class_name() const { return "RateCCR";};
  const char *processing() const { return "a/a"; };
  const char *flow_code()  const { return "-/-"; };

  TuplePtr simple_action(TuplePtr p);

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  class Connection;

  typedef std::map <ValuePtr, Connection*, Value::Comparator>  ValueConnectionMap;
  ValueConnectionMap  cmap_;

  DECLARE_PRIVATE_ELEMENT_INITS
};
  
#endif /* __RCCR_H_ */
