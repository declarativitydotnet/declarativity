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
#include "val_uint32.h"

class Route {
public:
  Route(ValuePtr k, ValuePtr l) : key_(k), location_(l) {}

  ValuePtr key_;
  ValuePtr location_;
};

SimpleKeyRouter::SimpleKeyRouter(str name)
  : Element(name,1, 1) { }


TuplePtr SimpleKeyRouter::simple_action(TupleRef tp)
{
  ValuePtr l = NULL;
  ValuePtr k = getKey(tp);
  for (std::vector<Route*>::iterator i = routes_.begin(); 
       i != routes_.end() &&  k->compareTo((*i)->key_) < 0; i++) {
    l = (*i)->location_;
  }

  TuplePtr p = Tuple::mk();
  p->append(l);
  for (uint i = 0; i < tp->size(); i++) 
    p->append((*tp)[i]);
  return p;
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
