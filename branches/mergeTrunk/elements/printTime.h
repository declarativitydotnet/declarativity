// -*- c-basic-offset: 2; related-file-name: "printTime.C" -*-
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

#ifndef __PRINTTIME_H__
#define __PRINTTIME_H__

#include "element.h"
#include "elementRegistry.h"

class PrintTime : public Element { 
public:

  PrintTime(string prefix);
  PrintTime(TuplePtr args);

  ~PrintTime();
  
  /** Overridden to perform the printing */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "PrintTime";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  /** The prefix to be placed on every printout by this element */
  string _prefix;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __PRINTTIME_H_ */
