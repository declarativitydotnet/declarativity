// -*- c-basic-offset: 2; related-file-name: "hexdump.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that dumps in hex the first element (it'd better
 * be a string) of the input tuple and outputs it in a single tuple.
 */

#ifndef __HEXDUMP_H__
#define __HEXDUMP_H__

#include "element.h"

class Hexdump : public Element { 
public:
  Hexdump();

  ~Hexdump();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TupleRef p);

  const char *class_name() const		{ return "Hexdump";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
};


#endif /* __HEXDUMP_H_ */
