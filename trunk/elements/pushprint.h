// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which simply prints any tuple pushed to it
 *
 */

#ifndef __PUSHPRINT_H__
#define __PUSHPRINT_H__

#include "element.h"

class PushPrint : public Element { 
public:
  
  PushPrint();
  
  int push(int port, TupleRef t, cbv cb);
  const char *class_name() const		{ return "PushPrint";}
  const char *processing() const		{ return "h/"; }
  const char *flow_code() const			{ return "/"; }
  
};


#endif /* __PUSHPRINT_H_ */
