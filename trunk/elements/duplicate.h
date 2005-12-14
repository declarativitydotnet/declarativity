// -*- c-basic-offset: 2; related-file-name: "duplicate.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: A duplicating push element.  It take tuples from its
 * single input and duplicates them on all of its outputs.  It pushes
 * back its input only if all of its outputs are backed up (the
 * block-all-block-all policy).
 */

#ifndef __DUPLICATE_H__
#define __DUPLICATE_H__

#include "element.h"
#include "bitvec.h"

class Duplicate : public Element { 
public:
  
  Duplicate(str, int);

  int push(int port, TupleRef t, b_cbv cb);

  const char *class_name() const		{ return "Duplicate";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const			{ return "x/x"; }

  /** Push back only if all outputs have pushed back. */
  int push(TupleRef p, b_cbv cb) const;

private:
  /** The callback for my input */
  b_cbv	_push_cb;

  /** My block flags, one per output port */
  bitvec _block_flags;

  /** My block flag count. */
  int _block_flag_count;

  /** My block callback function for a given output */
  void unblock(int output);
};


#endif /* __DUPLICATE_H_ */
