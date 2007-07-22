// -*- c-basic-offset: 2; related-file-name: "discard.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element which simply discards any tuples it is pushed.
 */

#ifndef __DISCARD_H__
#define __DISCARD_H__

#include "element.h"

class Discard : public Element { 
public:
  
  Discard(string name="discard");

  const char *class_name() const		{ return "Discard";}
  const char *processing() const		{ return "h/"; }
  const char *flow_code() const			{ return "-/"; }

  /** Overridden since I have no outputs */
  int push(int port, TuplePtr, b_cbv cb);
};


#endif /* __DISCARD_H_ */
