// -*- c-basic-offset: 2; related-file-name: "route.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that "routes" a string tuple.  It decides where to
 * send that string and prepends the destination in a sockaddr (embedded
 * into a str).
 */

#ifndef __ROUTE_H__
#define __ROUTE_H__

#include "element.h"
#include "value.h"
#include "fdbuf.h"

class Route : public Element { 
public:
  Route(string, FdbufPtr destinationUio);

  ~Route();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "Route";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** The destination address */
  ValuePtr _destination;
};


#endif /* __ROUTE_H_ */
