// -*- c-basic-offset: 2; related-file-name: "route.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
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

class Route : public Element { 
public:
  Route(str,
        ref< suio > destinationUio);

  ~Route();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TupleRef p);

  const char *class_name() const		{ return "Route";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** The destination address */
  ValueRef _destination;
};


#endif /* __ROUTE_H_ */
