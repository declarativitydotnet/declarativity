// -*- c-basic-offset: 2; related-file-name: "print.C" -*-
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
 * DESCRIPTION: Element which simply prints any tuple that passes
 * through it.
 */

#ifndef __IDENTITY_H__
#define __IDENTITY_H__

#include "element.h"
#include "elementRegistry.h"

class Identity : public Element { 
public:

  Identity(string name="identity");
  Identity(TuplePtr args);

  ~Identity();
  
  /** Overridden to perform the printing */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "Identity";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "-/-"; }


  DECLARE_PUBLIC_ELEMENT_INITS

private:

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __IDENTITY_H_ */
