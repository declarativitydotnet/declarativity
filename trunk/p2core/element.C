// -*- c-basic-offset: 2; related-file-name: "element.h" -*-
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
 * An element may be in one of two states: CONFIGURATION and RUNNING.
 * During configuration, the element maintains data about its ports
 * (including personalities, flow codes, etc.) that faciliate with
 * static analysis of the big picture of the forwarding engine.  Since
 * all those data aare not useful once the element is up and running, we
 * only keep them during configuration and discard them thereafter.
 */

#include "element.h"


// Some basic element types
const char * const Element::PUSH_TO_PULL = "h/l";
const char * const Element::PULL_TO_PUSH = "l/h";

// A basic flow code
const char * const Element::COMPLETE_FLOW = "x/x";


int Element::nelements_allocated = 0;
int Element::elementCounter = 0;

#if P2_STATS >= 2
# define ELEMENT_CTOR_STATS _calls(0), _self_cycles(0), _child_cycles(0),
#else
# define ELEMENT_CTOR_STATS
#endif

Element::Element() :
  ELEMENT_CTOR_STATS
  _inputs(0),
  _outputs(0),
  _ninputs(0),
  _noutputs(0)
{
  nelements_allocated++;
  _ID = elementCounter++;
}

Element::Element(int ninputs, int noutputs) :
  ELEMENT_CTOR_STATS
  _inputs(0),
  _outputs(0),
  _ninputs(0),
  _noutputs(0)
{
  set_nports(ninputs, noutputs);
  nelements_allocated++;
  _ID = elementCounter++;
}

Element::~Element()
{
  nelements_allocated--;
  if (_inputs)
    delete[] _inputs;
  if (_outputs)
    delete[] _outputs;
}

// INPUTS AND OUTPUTS

void
Element::set_nports(int new_ninputs, int new_noutputs)
{
  // exit on bad counts, or if already initialized
  // XXX initialized flag for element
  if (new_ninputs < 0 || new_noutputs < 0)
    return;
  
  // create new port arrays
  Port *new_inputs = new Port[new_ninputs];
  if (!new_inputs)		// out of memory -- return
    return;

  Port *new_outputs = new Port[new_noutputs];
  if (!new_outputs) {		// out of memory -- return
    delete[] new_inputs;
    return;
  }

  // install information
  if (_inputs)
    delete[] _inputs;
  if (_outputs)
    delete[] _outputs;
  _inputs = new_inputs;
  _outputs = new_outputs;
  _ninputs = new_ninputs;
  _noutputs = new_noutputs;
}

void
Element::set_ninputs(int count)
{
  set_nports(count, _noutputs);
}

void
Element::set_noutputs(int count)
{
  set_nports(_ninputs, count);
}

bool
Element::ports_frozen() const
{
  assert(0);                    // Deal with freezing without port0
  return false;
}

void
Element::initialize_ports(const int *in_v, const int *out_v)
{
  // Only PULL inputs are connectable
  for (int i = 0; i < ninputs(); i++) {
    // allowed iff in_v[i] == VPULL
    int port = (in_v[i] == PULL ? 0 : Port::NOT_CONNECTABLE);
    _inputs[i] = Port(this, 0, port);
  }
  
  for (int o = 0; o < noutputs(); o++) {
    // allowed iff out_v[o] != VPULL
    int port = (out_v[o] == PULL ? Port::NOT_CONNECTABLE : 0);
    _outputs[o] = Port(this, 0, port);
  }
}

int Element::connect_input(int i, Element *f, int port)
{
  if (i >= 0 && i < ninputs() && _inputs[i].allowed()) {
    _inputs[i] = Port(this, f, port);
    return 0;
  } else
    return -1;
}

int Element::connect_output(int o, Element *f, int port)
{
  if (o >= 0 && o < noutputs() && _outputs[o].allowed()) {
    _outputs[o] = Port(this, f, port);
    return 0;
  } else
    return -1;
}

// PUSH OR PULL PROCESSING

const char * Element::processing() const
{
  return "a/a";
}

const char *Element::flow_code() const
{
  return COMPLETE_FLOW;
}

const char *Element::flags() const
{
  return "";
}

#if P2_STATS >= 1

static str
read_icounts_handler(Element *f, void *)
{
  strbuf sa;
  for (int i = 0; i < f->ninputs(); i++)
    if (f->input(i).allowed() || P2_STATS >= 2)
      sa << f->input(i).ntuples() << "\n";
    else
      sa << "??\n";
  return sa.take_string();
}

static str
read_ocounts_handler(Element *f, void *)
{
  strbuf sa;
  for (int i = 0; i < f->noutputs(); i++)
    if (f->output(i).allowed() || P2_STATS >= 2)
      sa << f->output(i).ntuples() << "\n";
    else
      sa << "??\n";
  return sa.take_string();
}

#endif /* P2_STATS >= 1 */

#if P2_STATS >= 2
/*
 * cycles:
 * # of calls to this element (push or pull).
 * cycles spent in this element and elements it pulls or pushes.
 * cycles spent in the elements this one pulls and pushes.
 */
static str
read_cycles_handler(Element *f, void *)
{
  return(str(f->_calls) + "\n" +
         str(f->_self_cycles) + "\n" +
         str(f->_child_cycles) + "\n");
}
#endif


// RUNNING

int Element::push(int port, TupleRef p, cbv cb)
{
  p = simple_action(p);
  if (p) return output(0).push(p,cb);
  return 1;
}

TuplePtr Element::pull(int port, cbv cb)
{
  TuplePtr p = input(0).pull(cb);
  if (p) p = simple_action(p);
  return p;
}

TupleRef Element::simple_action(TupleRef p)
{
  return p;
}





















REMOVABLE_INLINE const Element::Port & Element::input(int i) const
{
  assert(i >= 0 && i < ninputs());
  return _inputs[i];
}

REMOVABLE_INLINE const Element::Port &
Element::output(int o) const
{
  assert(o >= 0 && o < noutputs());
  return _outputs[o];
}

REMOVABLE_INLINE bool
Element::output_is_push(int o) const
{
  return o >= 0 && o < noutputs() && _outputs[o].allowed();
}

REMOVABLE_INLINE bool
Element::output_is_pull(int o) const
{
  return o >= 0 && o < noutputs() && !_outputs[o].allowed();
}

REMOVABLE_INLINE bool
Element::input_is_pull(int i) const
{
  return i >= 0 && i < ninputs() && _inputs[i].allowed();
}

REMOVABLE_INLINE bool
Element::input_is_push(int i) const
{
  return i >= 0 && i < ninputs() && !_inputs[i].allowed();
}

////////////////////////////////////////////////////////////
// Element::Port


REMOVABLE_INLINE int Element::Port::push(TupleRef p, cbv cb) const
{
  // If I am not connected, I shouldn't be pushed.
  assert(_e);
#if P2_STATS >= 1
  _tuples++;
#endif

  int returnValue;
#if P2_STATS >= 2
  _e->input(_port)._tuples++;
  uint64_t c0 = click_get_cycles();
#endif // P2_STATS >= 2

  returnValue = _e->push(_port, p, cb);

#if P2_STATS >= 2
  uint64_t c1 = click_get_cycles();
  uint64_t x = c1 - c0;
  _e->_calls += 1;
  _e->_self_cycles += x;
  _owner->_child_cycles += x;
#endif

  return returnValue;
}

REMOVABLE_INLINE TuplePtr Element::Port::pull(cbv cb) const
{
  // If I am not connected, I shouldn't be pulled.
  assert(_e);
#if P2_STATS >= 2
  _e->output(_port)._tuples++;
  uint64_t c0 = click_get_cycles();
#endif

  TuplePtr p = _e->pull(_port, cb);

#if P2_STATS >= 2
  uint64_t c1 = click_get_cycles();
  uint64_t x = c1 - c0;
  _e->_calls += 1;
  _e->_self_cycles += x;
  _owner->_child_cycles += x;
#endif

#if P2_STATS >= 1
  if (p) _tuples++;
#endif

  return p;
}

/** Construct a detached free port */
REMOVABLE_INLINE Element::Port::Port() :
  _e(0),
  _port(NOT_INITIALIZED),
  _cb(cbv_null)
  PORT_CTOR_INIT(0)
{ }

/** Construct an attached port */
REMOVABLE_INLINE Element::Port::Port(Element *owner,
                                     Element *e,
                                     int p)
  : _e(e),
    _port(p),
    _cb(cbv_null)
  PORT_CTOR_INIT(owner)
{
  (void) owner;
}

