// -*- c-basic-offset: 2; related-file-name: "discard.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which simply discards any tuples it is pushed.
 */

#ifndef __DISCARD_H__
#define __DISCARD_H__

#include "element.h"

class Discard : public Element { 
public:
  
  Discard();

  const char *class_name() const		{ return "Discard";}
  const char *processing() const		{ return "l/"; }
  const char *flow_code() const			{ return "-/"; }

  /** Overridden since I have no outputs */
  int push(int port, TupleRef, cbv cb);

private:
};


#endif /* __DISCARD_H_ */
