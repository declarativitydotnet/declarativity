// -*- c-basic-offset: 2; related-file-name: "mux.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: A round-robin multiplexing pull element.  Whenever a
 * pull is requested, each of its inputs is pulled in order, unless it
 * is blocked.  If all inputs are blocked then the output is blocked as
 * well.
 */

#ifndef __MUX_H__
#define __MUX_H__

#include "element.h"
#include "bitvec.h"

class Mux : public Element { 
public:
  
  Mux(str, int);

  TuplePtr pull(int port, cbv cb);

  const char *class_name() const		{ return "Mux";}
  const char *processing() const		{ return "l/l"; }
  const char *flow_code() const			{ return "x/x"; }

  /** A tuple may be dropped without notification if it resolves to an
      output that's held back. */
  int push(TupleRef p, cbv cb) const;

private:
  /** The callback for my outputs */
  cbv	_pull_cb;

  /** My block flags, one per input port */
  bitvec _block_flags;

  /** My block flag count. */
  int _block_flag_count;

  /** My current input pointer */
  int _nextInput;

  /** My block callback function for a given input */
  void unblock(int input);
};


#endif /* __MUX_H_ */
