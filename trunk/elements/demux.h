// -*- c-basic-offset: 2; related-file-name: "demux.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: A demultiplexing element.  It checks tuples' first field
 * for equality with the registered key for its outputs and routes
 * tuples accordingly, including the demux key.  It pushes back its
 * input only if all of its outputs are backed up (the
 * block-all-block-all policy).  If two or more demux keys are equal,
 * only the first receives a tuple; for duplication an explicit
 * duplicator element should be used.
 */

#ifndef __DEMUX_H__
#define __DEMUX_H__

#include "element.h"
#include "bitvec.h"

class Demux : public Element { 
public:
  
  Demux(ref< vec< ValueRef > >);

  int push(int port, TupleRef t, cbv cb);

  const char *class_name() const		{ return "Demux";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const			{ return "x/x"; }

  /** A tuple may be dropped without notification if it resolves to an
      output that's held back. */
  int push(TupleRef p, cbv cb) const;

private:
  /** The callback for my input */
  cbv	_push_cb;

  /** My demux key vector */
  ref< vec< ValueRef > > _demuxKeys;

  /** My block flags, one per output port */
  bitvec _block_flags;

  /** My block flag count. */
  int _block_flag_count;

  /** My block callback function for a given output */
  void unblock(int output);
};


#endif /* __DEMUX_H_ */
