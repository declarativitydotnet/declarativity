// -*- c-basic-offset: 2; related-file-name: "noNullField.C" -*-
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
 * DESCRIPTION: Element that only passes through tuples whose named
 * field is a non-empty tuple.
 */

#ifndef __NONULLFIELD_H__
#define __NONULLFIELD_H__

#include "element.h"
#include "elementRegistry.h"

class NoNullField : public Element { 
public:
  NoNullField(string, unsigned);
  NoNullField(TuplePtr args);
  ~NoNullField();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "NoNullField";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  /** The field number to check */
  unsigned _fieldNo;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __NONULLFIELD_H_ */
