// -*- c-basic-offset: 2; related-file-name: "hexdump.C" -*-
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
 * DESCRIPTION: Element that dumps in hex the named opaque element of
 * the input tuple.  It just replaces that field in the tuple which it
 * then outputs.
 */

#ifndef __HEXDUMP_H__
#define __HEXDUMP_H__

#include "element.h"
#include "elementRegistry.h"

class Hexdump : public Element { 
public:
  Hexdump(string, unsigned);
  Hexdump(TuplePtr args);

  ~Hexdump();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "Hexdump";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


  DECLARE_PUBLIC_ELEMENT_INITS

private:
  /** My field number to hexdump */
  unsigned _fieldNo;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __HEXDUMP_H_ */
