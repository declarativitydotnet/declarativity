/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: This tuple source allows the owner of the object to push
 * tuples into the dataflow. The interface to the non-P2 user of this
 * element is one in which the tuple(TuplePtr) 
 */

#ifndef __TUPLE_INJECTOR_H__
#define __TUPLE_INJECTOR_H__

#include "element.h"
#include "elementRegistry.h"

class TupleInjector : public Element
{ 
public:


  /** Same construction as element. */
  TupleInjector(string);


  /** Tuple-based constructor */
  TupleInjector(TuplePtr args);


  const char*
  class_name() const { return "TupleInjector";}


  const char*
  processing() const { return "/h"; }


  const char*
  flow_code() const { return "/-"; }


  /** Similar to Element::push. Accept a tuple for processing unless
      blocked and call the callback when the blocking away.*/
  int
  tuple(TuplePtr, b_cbv cb);


  DECLARE_PUBLIC_ELEMENT_INITS




private:

  DECLARE_PRIVATE_ELEMENT_INITS
};

typedef boost::shared_ptr< TupleInjector > TupleInjectorPtr;

#endif /* __TUPLE_INJECTOR_H_ */
