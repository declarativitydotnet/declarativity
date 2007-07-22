// -*- c-basic-offset: 2; related-file-name: "duplicate.C" -*-
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
 * DESCRIPTION: A duplicating push element.  It take tuples from its
 * single input and duplicates them on all of its outputs.  It pushes
 * back its input only if all of its outputs are backed up (the
 * block-all-block-all policy).
 */

#ifndef __DUPLICATE_H__
#define __DUPLICATE_H__

#include "element.h"

class Duplicate : public Element { 
public:
  
  Duplicate(string, int);

  int push(int port, TuplePtr t, b_cbv cb);

  const char *class_name() const		{ return "Duplicate";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const			{ return "x/x"; }

  /** Push back only if all outputs have pushed back. */
  int push(TuplePtr p, b_cbv cb) const;

private:
  /** The callback for my input */
  b_cbv	_push_cb;

  /** My block flags, one per output port */
  std::vector<int> _block_flags;

  /** My block flag count. */
  unsigned _block_flag_count;

  /** My block callback function for a given output */
  void unblock(unsigned output);
};


#endif /* __DUPLICATE_H_ */
