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
#include "inlines.h"

class RateCCR : public Element {
public:
  RateCCR(string name);
  const char *class_name() const { return "RateCCR";};
  const char *processing() const { return "a/a"; };
  const char *flow_code()  const { return "-/-"; };

  TuplePtr simple_action(TuplePtr p);

private:
  class Connection;

  typedef std::map <ValuePtr, Connection*, Value::Less>  ValueConnectionMap;
  ValueConnectionMap  cmap_;
};
  
#endif /* __RCCR_H_ */
