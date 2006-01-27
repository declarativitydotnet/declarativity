// -*- c-basic-offset: 2; related-file-name: "mux.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A multiplexing push element.  A push in any of the
 * inputs results in push at the single output.  If the output is
 * blocked, all inputs are blocked.  The element queues one tuple per
 * input since in P2, elements don't offer unsolicited blockage info.
 * In other words, if input 0 causes a block on the mux element, input 1
 * has no way to find out other than failing to push something.
 */

#ifndef __MUX_H__
#define __MUX_H__

#include "element.h"
#include <vector>

class Mux : public Element { 
public:
  
  Mux(string, int);
  ~Mux() {}

  const char *class_name() const		{ return "Mux";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const			{ return "-/-"; }

  int push(int, TuplePtr, b_cbv);

  /** My callback method */
  void callback();

  /** My catch up method, to handle callbacks off the callback thread */
  void catchUp();

  /** Add a new input port, and return the port number */
  int add_input();

  /** Remove port (will not affect other port positions) */
  void remove_input(int);

private:
  bool isUnusedPort(int port);

  /** Is my output blocked? */
  bool _blocked;

  /** Unused port numbers */
  std::vector<int> _unusedPorts;

  /** My input callback vector */
  std::vector< b_cbv > _pushCallbacks;

  /** My input buffer, one tuple per input. */
  std::vector< TuplePtr > _inputTuples;

  /** My catch-up callback */
  b_cbv _catchUp;

  /** My time callback */
  timeCBHandle * _timeCallback;

  /** My own output callback */
  b_cbv _callback;
};


#endif /* __MUX_H_ */
