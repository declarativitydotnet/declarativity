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
 * DESCRIPTION: Abstract class for all 0-in-1-out elements that return
 * tuples when pulled.
 */


#ifndef __TUPLE_SOURCE_H__
#define __TUPLE_SOURCE_H__

#include "element.h"

class TupleSource : public Element { 
public:
  
  /** To construct, just give the name of the element object and a
      generator object, whose ownership passes over to the element (so
      the pointer should no longer be used after the constructor has
      been invoked). */
  TupleSource(string);


  TupleSource(TuplePtr args);


  const char*
  class_name() const { return "TupleSource"; }


  const char*
  flow_code() const { return "/-"; }


  const char*
  processing() const { return "/l"; }


  virtual TuplePtr
  pull(int port, b_cbv cb);




protected:
  /** The generator function for this tuple source. Always returns a
      tuple (cannot refuse). */
  virtual TuplePtr
  generate() = 0;


};

#endif /* __TUPLE_SOURCE_H_ */
