// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
/*
 * @(#)$Id$
 *
 * Modified from the Click Element base class by Eddie Kohler
 * statistics: Robert Morris
 * P2 version: Timothy Roscoe
 * 
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2004 Regents of the University of California
 * Copyright (c) 2004 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Base class for P2 elements
 *
 * Note to P2 hackers: much has been removed from the original Click
 * class.  Much of that may well find its way back in, but for now
 * let's put stuff back in when we understand why we need it.  That
 * way we'll all develop a better understanding of why Click (and P2)
 * are the way they are. 
 * 
 * We've also enhanced to push/pull tuple to allow blocking (something
 * Click, being a router, doesn't need to bother with). 
 */

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "tuple.h"

class Element { 
public:
  
  // Port types.  These are the constituent characters of the
  // "processing" signature of the element. 
  static const char * const AGNOSTIC, * const PUSH, * const PULL;
  // Two shorthand processing signatures.  
  static const char * const PUSH_TO_PULL, * const PULL_TO_PUSH;
  enum Processing { VAGNOSTIC, VPUSH, VPULL };

  class Port;
  
  Element();
  Element(int ninputs, int noutputs);
  virtual ~Element();
  static int nelements_allocated;

  //
  // RUNTIME
  //

  // If push returns '1', it's OK to send more tuples, and the
  // callback has not been registered.  If '0', it's NOT OK to send
  // more tuples, and the callback will be invoked as soon as it is. 
  virtual int push(int port, TupleRef, cbv cb);

  // If pull returns a Tuple, the callback has not been registered and
  // there _might_ be another Tuple available.  If it returns null,
  // there wasn't another Tuple, and the callback will be invoked when
  // there is another one. 
  virtual TuplePtr pull(int port, cbv cb);

  virtual TupleRef simple_action(TupleRef p);

  // CHARACTERISTICS
  virtual const char *class_name() const = 0;

  // INPUTS AND OUTPUTS
  int ninputs() const				{ return _ninputs; }
  int noutputs() const				{ return _noutputs; }
  void set_ninputs(int);
  void set_noutputs(int);
  void add_input()				{ set_ninputs(ninputs()+1); }
  void add_output()				{ set_noutputs(noutputs()+1); }
  bool ports_frozen() const;

  void initialize_ports(const int *in_v, const int *out_v);
  int connect_input(int i, Element *f, int port);
  int connect_output(int o, Element *f, int port);
  
  const Port &input(int) const;
  const Port &output(int) const;

  bool input_is_push(int) const;
  bool input_is_pull(int) const;
  bool output_is_push(int) const;
  bool output_is_pull(int) const;
  
  void checked_output_push(int port, TupleRef t, cbv cb) const;

  // PROCESSING, FLOW, AND FLAGS
  virtual const char *processing() const;
  virtual const char *flow_code() const;
  virtual const char *flags() const;
  
  class Port { 
  public:

    Port();
    Port(Element *, Element *, int);
    
    operator bool() const		{ return _e != 0; }
    bool allowed() const		{ return _port >= 0; }
    bool initialized() const		{ return _port >= -1; }
    
    Element *element() const		{ return _e; }
    int port() const			{ return _port; }
    
    int push(TupleRef p, cbv cb) const;
    TuplePtr pull(cbv cb) const;

#if P2_STATS >= 1
    unsigned ntuples() const		{ return _tuples; }
#endif

   private:
    
    Element *_e;
    int _port;
    cbv _cb;
    
#if P2_STATS >= 1
    mutable unsigned _tuples;		// How many tuples have we moved?
#endif
#if P2_STATS >= 2
    Element *_owner;			// Whose input or output are we?
#endif
    
  };

 private:

  enum { INLINE_PORTS = 4 };

  Port *_inputs;
  Port *_outputs;
  Port _ports0[INLINE_PORTS];

  int _ninputs;
  int _noutputs;

  Element(const Element &);
  Element &operator=(const Element &);
  
  void set_nports(int, int);
  
};


inline const Element::Port &
Element::input(int i) const
{
  assert(i >= 0 && i < ninputs());
  return _inputs[i];
}

inline const Element::Port &
Element::output(int o) const
{
  assert(o >= 0 && o < noutputs());
  return _outputs[o];
}

inline bool
Element::output_is_push(int o) const
{
  return o >= 0 && o < noutputs() && _outputs[o].allowed();
}

inline bool
Element::output_is_pull(int o) const
{
  return o >= 0 && o < noutputs() && !_outputs[o].allowed();
}

inline bool
Element::input_is_pull(int i) const
{
  return i >= 0 && i < ninputs() && _inputs[i].allowed();
}

inline bool
Element::input_is_push(int i) const
{
  return i >= 0 && i < ninputs() && !_inputs[i].allowed();
}

#if P2_STATS >= 2
# define PORT_CTOR_INIT(o) , _tuples(0), _owner(o)
#else
# if P2_STATS >= 1
#  define PORT_CTOR_INIT(o) , _tuples(0)
# else
#  define PORT_CTOR_INIT(o)
# endif
#endif

inline Element::Port::Port() : _e(0), _port(-2), _cb(cbv_null) PORT_CTOR_INIT(0) { }

inline Element::Port::Port(Element *owner, Element *e, int p)
  : _e(e), _port(p), _cb(cbv_null) PORT_CTOR_INIT(owner)
{
  (void) owner;
}

inline int Element::Port::push(TupleRef p, cbv cb) const
{
  assert(_e);
#if P2_STATS >= 1
  _tuples++;
#endif
#if P2_STATS >= 2
  _e->input(_port)._tuples++;
  uint64_t c0 = click_get_cycles();
  _e->push(_port, p, cb);
  uint64_t c1 = click_get_cycles();
  uint64_t x = c1 - c0;
  _e->_calls += 1;
  _e->_self_cycles += x;
  _owner->_child_cycles += x;
#else
  _e->push(_port, p, cb);
#endif
}

inline TuplePtr Element::Port::pull(cbv cb) const
{
  assert(_e);
#if P2_STATS >= 2
  _e->output(_port)._tuples++;
  uint64_t c0 = click_get_cycles();
  TuplePtr p = _e->pull(_port, cb);
  uint64_t c1 = click_get_cycles();
  uint64_t x = c1 - c0;
  _e->_calls += 1;
  _e->_self_cycles += x;
  _owner->_child_cycles += x;
#else
  TuplePtr p = _e->pull(_port, cb);
#endif
#if P2_STATS >= 1
  if (p) _tuples++;
#endif
  return p;
}

inline void
Element::checked_output_push(int o, TupleRef p, cbv cb) const
{
  if ((unsigned)o < (unsigned)noutputs()) {
    _outputs[o].push(p, cb);
    } else {
    // p->kill();
    }
}

#endif /* __ELEMENT_H_ */
