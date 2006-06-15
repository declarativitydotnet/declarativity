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

#ifndef __TUPLESOURCEINTERFACE_H__
#define __TUPLESOURCEINTERFACE_H__

#include "element.h"

class TupleSourceInterface : public Element { 
public:

  TupleSourceInterface(string);

  const char *class_name() const	{ return "TupleSourceInterface";}
  const char *processing() const	{ return "/h"; }
  const char *flow_code() const		{ return "/-"; }

  int tuple(TuplePtr);

private:
  void unblock();

  int _notBlocked;
};


#endif /* __TUPLESOURCEINTERFACE_H_ */
