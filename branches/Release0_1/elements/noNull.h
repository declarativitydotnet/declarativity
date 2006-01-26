// -*- c-basic-offset: 2; related-file-name: "noNull.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that only passes through non-empty tuples
 */

#ifndef __NONULL_H__
#define __NONULL_H__

#include "element.h"

class NoNull : public Element { 
public:
  NoNull(string);
  ~NoNull();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "NoNull";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }
};


#endif /* __NONULL_H_ */
