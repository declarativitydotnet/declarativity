// -*- c-basic-offset: 2; related-file-name: "print.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: Element which simply prints any tuple that passes
 * through it.
 */

#ifndef __PRINT_H__
#define __PRINT_H__

#include "element.h"

class Print : public Element { 
public:

  Print(string prefix);

  ~Print();
  
  /** Overridden to perform the printing */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "Print";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }

private:
  /** The prefix to be placed on every printout by this element */
  string _prefix;
};


#endif /* __PRINT_H_ */
