// -*- c-basic-offset: 2; related-file-name: "queue.C" -*-
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
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "element.h"
#include <queue>

/** Simple queue that buffers up elements pushed in as input, and pulled from the other end. */
class Queue : public Element { 
public:

  Queue(string name="queue", unsigned int queueSize=0);

  ~Queue();
  
  /** Overridden to perform the printing */
  int push(int port, TuplePtr p, b_cbv cb);
  TuplePtr pull(int port, b_cbv cb);  


  const char *class_name() const		{ return "Queue";}
  const char *processing() const		{ return "h/l"; }
  const char *flow_code() const			{ return "x/x"; }

private:
  b_cbv _pullCB, _pushCB;
  unsigned int _size;
  /** The tuple ref array from which I pull */
  std::queue<TuplePtr> _q;
};


#endif /* __PRINT_H_ */
