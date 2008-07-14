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

  SimpleKeyRouter(str name, ValuePtr id, bool retry=false);
  const char *class_name() const	{ return "SimpleKeyRouter";};
  const char *processing() const	{ return "hh/hh"; };
  const char *flow_code() const		{ return "-/-"; };

  int push(int port, TupleRef tp, cbv cb);

  void route(ValuePtr key, ValuePtr loc);

  ref< vec< ValueRef > > neighbors();
  ref< vec< ValueRef > > routes();

private:
  REMOVABLE_INLINE int greedyRoute(TuplePtr tp);
  REMOVABLE_INLINE TuplePtr tagRoute(TuplePtr tp, uint r);
  REMOVABLE_INLINE TuplePtr untagRoute(TuplePtr tp);
  REMOVABLE_INLINE int getRoute(TuplePtr tp);
  REMOVABLE_INLINE ValuePtr getKey(TuplePtr tp);

  ValuePtr my_id_;
  bool     retry_;
  std::vector<Route*> routes_;
};

#endif /* __SimpleNetSim_H_ */
