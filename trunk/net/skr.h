// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#ifndef __SKR_H__
#define __SKR_H__

#include <vector>
#include "element.h"

class Route;

class SimpleKeyRouter : public Element {
public:

  SimpleKeyRouter(str name="SimpleKeyRouter");
  const char *class_name() const	{ return "SimpleKeyRouter";};
  const char *processing() const	{ return "ah/ah"; };
  const char *flow_code() const		{ return "--/--"; };

  TuplePtr simple_action(TupleRef p);

  void route(ValuePtr key, ValuePtr loc);

  ref< vec< ValueRef > > neighbors();

private:
  REMOVABLE_INLINE ValuePtr getKey(TuplePtr tp);
  std::vector<Route*> routes_;
};

#endif /* __SimpleNetSim_H_ */
