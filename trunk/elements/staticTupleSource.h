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
 * DESCRIPTION: A tuple source that always returns the same tuple, given
 * at construction time.
 */


#ifndef __STATIC_TUPLE_SOURCE_H__
#define __STATIC_TUPLE_SOURCE_H__

#include "element.h"
#include "elementRegistry.h"
#include <boost/shared_ptr.hpp>
#include "tupleSource.h"

class StaticTupleSource : public TupleSource { 
public:
  
  DECLARE_PUBLIC_ELEMENT_INITS

  /** Given the element name and the tuple to be returning */
  StaticTupleSource(string, TuplePtr);


  StaticTupleSource(TuplePtr args);


  const char *class_name() const		{ return "StaticTupleSource"; }
  const char *flow_code() const			{ return "/-"; }
  const char *processing() const		{ return "/l"; }


protected:
  /** Override the generate virtual function to return the static
      tuple */
  TuplePtr
  generate();
  


private:
  /** My tuple */
  TuplePtr _tuple;
    
  DECLARE_PRIVATE_ELEMENT_INITS
};

#endif /* __STATIC_TUPLE_SOURCE_H_ */
