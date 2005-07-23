// -*- cr-basic-offset: 2; related-file-name: "tupleseq.h" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include <iostream>
#include "skr.h"
#include "val_tuple.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_uint32.h"

class Route {
public:
  Route(ValuePtr k, ValuePtr l) : key_(k), location_(l) {}

  ValuePtr key_;
  ValuePtr location_;
};

SimpleKeyRouter::SimpleKeyRouter(str n, bool r) 
  : Element(n, 2, 1), retry_(r) { }


int SimpleKeyRouter::push(int port, TupleRef tp, cbv cb) {
  int route = 0;

  if (port == 0) route = greedyRoute(tp);
  else if (port == 1) {
    bool failure = false;
    try {
      if (Val_Str::cast((*tp)[0]) == "FAIL") {
        tp = Val_Tuple::cast((*tp)[1]); 
        failure = true;
      }
    }
    catch (Value::TypeError& e) { } 
    tp = untagRoute(tp);
    if (failure) {
      route = getRoute(tp) - 1;			// Try a new route
      if (route < 0) route = greedyRoute(tp); 	// Oops, start over
    }
    else route = greedyRoute(tp);		// New data tuple or forwarding
    tp = untagRoute(tp);
  }

  assert(output(0)->push(tagRoute(tp, route), cb));

  return 1;
}

void SimpleKeyRouter::route(ValuePtr key, ValuePtr loc) {
  std::vector<Route*>::iterator i = routes_.begin();
  for ( ; i != routes_.end() && key->compareTo((*i)->key_) < 0; i++)
    ;
  routes_.insert(i, new Route(key, loc));
}

ref< vec< ValueRef > > SimpleKeyRouter::neighbors() {
  ref< vec< ValueRef > > n = New refcounted< vec< ValueRef > >;
  for (std::vector<Route*>::iterator i = routes_.begin(); 
       i != routes_.end(); i++) n->push_back((*i)->key_);
  return n;
}

REMOVABLE_INLINE uint SimpleKeyRouter::greedyRoute(TuplePtr tp) 
{
  uint route = 0;
  ValuePtr k = getKey(tp);
  for (std::vector<Route*>::iterator i = routes_.begin(); 
       i != routes_.end() &&  k->compareTo((*i)->key_) < 0; i++, route++)
    ;
  return route;
}

REMOVABLE_INLINE TuplePtr SimpleKeyRouter::tagRoute(TuplePtr tp, uint r) {
  TuplePtr tuple = Tuple::mk();
  tuple->append(routes_[r]->location_);

  TuplePtr tag   = Tuple::mk();
  tag->append(Val_Str::mk("ROUTE"));
  tag->append(Val_UInt32::mk(r));
  tag->freeze();
  tuple->append(Val_Tuple::mk(tag));
  for (uint i = 0; i < tp->size(); i++)
    tuple->append((*tp)[i]); 
  tuple->freeze();
  return tuple;
}

REMOVABLE_INLINE TuplePtr SimpleKeyRouter::untagRoute(TuplePtr tp) {
  TuplePtr tuple = Tuple::mk();
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TupleRef t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "ROUTE") continue;
    }
    catch (Value::TypeError& e) { } 
    tuple->append((*tp)[i]);
  }
  return tuple;
}

REMOVABLE_INLINE int SimpleKeyRouter::getRoute(TuplePtr tp) {
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TupleRef t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "ROUTE") return Val_Int32::cast((*t)[1]);
    }
    catch (Value::TypeError& e) { } 
  }
  return -1;
}

REMOVABLE_INLINE ValuePtr SimpleKeyRouter::getKey(TuplePtr tp) {
  for (uint i = 0; i < tp->size(); i++) {
    try {
      TupleRef t = Val_Tuple::cast((*tp)[i]); 
      if (Val_Str::cast((*t)[0]) == "KEY") {
        return (*t)[1];
      }
    }
    catch (Value::TypeError& e) { } 
  }
  return 0;
}
