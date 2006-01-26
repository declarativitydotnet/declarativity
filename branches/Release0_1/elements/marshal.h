// -*- c-basic-offset: 2; related-file-name: "marshal.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that marshals the input tuple into a string and
 * creates an output tuple containing that string.
 */

#ifndef __MARSHAL_H__
#define __MARSHAL_H__

#include "element.h"

class Marshal : public Element { 
public:
  Marshal(string);

  ~Marshal();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "Marshal";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
};


#endif /* __MARSHAL_H_ */
