// -*- c-basic-offset: 2; related-file-name: "print.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element which simply prints any tuple that passes
 * through it.
 */

#ifndef __PRINT_H__
#define __PRINT_H__

#include "element.h"

class Print : public Element { 
public:

  Print();

  ~Print();
  
  int push(int port, TupleRef t, cbv cb);
  TuplePtr pull(int port, cbv);

  const char *class_name() const		{ return "Print";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }

private:
  
};


#endif /* __PRINT_H_ */
