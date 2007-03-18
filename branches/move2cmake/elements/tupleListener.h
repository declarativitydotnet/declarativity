/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#ifndef __TUPLELISTENER_H__
#define __TUPLELISTENER_H__

#include "element.h"
#include <map>

typedef boost::function<void (TuplePtr)> cb_tp;

class TupleListener : public Element { 
public:

  TupleListener(string, cb_tp);

  int push(int, TuplePtr, b_cbv);

  const char *class_name() const	{ return "TupleListener";}
  const char *processing() const	{ return "a/"; }
  const char *flow_code() const		{ return "x/"; }

private:
  cb_tp _callback;
};


#endif /* __TUPLELISTENER_H_ */
