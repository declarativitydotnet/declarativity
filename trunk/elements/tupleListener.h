/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: The tuple listener element invokes a callback of type
 * void(TuplePtr) with any tuple that is pushed to its input.  Control
 * is yielded back to P2 only after the callback has returned.
 */

#ifndef __TUPLELISTENER_H__
#define __TUPLELISTENER_H__

#include "element.h"

class TupleListener : public Element { 
public:

  /** The type of expected callbacks */
  typedef boost::function< void (TuplePtr) > TupleCallback;


  /** Create a listener given the element name and the specific
      callback */
  TupleListener(std::string,
                TupleCallback);


  /** Override push to invoke callback with input tuple */
  int
  push(int, TuplePtr, b_cbv);

  
  const char *class_name() const	{ return "TupleListener";}


  const char *processing() const	{ return "h/"; }


  const char *flow_code() const		{ return "-/"; }


private:

  /** My registered callback */
  TupleCallback _callback;
};


#endif /* __TUPLELISTENER_H_ */
