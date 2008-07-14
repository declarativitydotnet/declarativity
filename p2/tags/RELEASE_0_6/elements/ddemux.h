// -*- c-basic-offset: 2; related-file-name: "ddemux.C" -*-
/*
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: A dynamic demultiplexing element.  It checks tuples' input field
 * (the first by default) for equality with the registered key for its
 * outputs and routes tuples accordingly, including the demux key.  It
 * pushes back its input only if all of its outputs are backed up (the
 * block-all-block-all policy).  If two or more demux keys are equal,
 * then this thing bombs out (assertion); for duplication an explicit
 * duplicator element should be used.  The element contains one extra
 * output (on port 0) for the "else" case, i.e., for input that matches no key.
 */

#ifndef __DDEMUX_H__
#define __DDEMUX_H__

#include <map>
#include <vector>
#include "element.h"

class DDemux : public Element { 
public:
  
  DDemux(string, std::vector< ValuePtr >, unsigned f = 0);

  int push(int port, TuplePtr t, b_cbv cb);

  const char *class_name() const		{ return "DDemux";}
  const char *processing() const		{ return "h/h"; }
  const char *flow_code() const			{ return "-/-"; }

  /** A tuple may be dropped without notification if it resolves to an
      output that's held back.  Push back only if all outputs have
      pushed back. */
  int push(TuplePtr p, b_cbv cb) const;

  /** Add output port keyed off ValuePtr argument, returns allocated port # */
  int add_output(ValuePtr);

  /** Remove output port, by port # or key */
  void remove_output(ValuePtr);

private:
  /** The callback for my input */
  b_cbv	_push_cb;

  typedef std::map<ValuePtr, int> PortMap;
  PortMap _port_map;

  /** Place holder for removed ports */
  std::vector<int> _unusedPorts;

  /** My block flags, one per output port */
  std::vector<bool> _block_flags;

  /** My block flag count. */
  int _block_flag_count;

  /** My block callback function for a given output */
  void unblock(int output);

  /** The input field on which I perform the demultiplexing */
  unsigned _inputFieldNo;
};


#endif /* __DDEMUX_H_ */

