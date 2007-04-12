// -*- c-basic-offset: 2; related-file-name: "demux.C" -*-
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
 * DESCRIPTION: A conservatively blocked demultiplexing element.  It
 * checks tuples' input field (the first by default) for equality with
 * the registered key for its outputs and routes tuples accordingly,
 * including the demux key.  It pushes back its input if any of its
 * outputs are backed up (the block-one-block-all policy).  If two or
 * more demux keys are equal, only the first receives a tuple; for
 * duplication an explicit duplicator element should be used.  The
 * element contains one extra output for the "else" case, i.e., for
 * input that matches no key.
 */

#ifndef __DEMUXCONSERVATIVE_H__
#define __DEMUXCONSERVATIVE_H__

#include "element.h"

class DemuxConservative : public Element { 
public:
  /** Create a conservative demux given its name, its demux keys, and
      the field number on which to apply the demux function */
  DemuxConservative(string,
                    std::vector< ValuePtr >,
                    unsigned f = 0);

  int
  push(int port, TuplePtr t, b_cbv cb);


  const char *class_name() const		{ return "DemuxConservative";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const			{ return "-/-"; }


  /** Override Element::toDot to show names of outputs */
  void
  toDot(std::ostream*);


  /** My demux key vector */
  std::vector< ValuePtr > _demuxKeys;




private:
  /** The callback for my input */
  b_cbv	_push_cb;


  /** My blocked output port if any. If I am not blocked, this is -1. */
  int _blockedOutput;


  /** My block callback function for a given output */
  void
  unblock(unsigned output);


  /** The input field on which I perform the demultiplexing */
  unsigned _inputFieldNo;
};


#endif /* __DEMUXCONSERVATIVE_H_ */
