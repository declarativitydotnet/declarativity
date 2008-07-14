// -*- c-basic-offset: 2; related-file-name: "element.h" -*-
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

#include <errno.h>
#include "element.h"
#include "plumber.h"
#include "loggerI.h"
#include "p2Time.h"

#include "val_double.h"
#include "val_uint64.h"
#include "val_str.h"
#include "val_int32.h"
#include "val_time.h"
#include "reporting.h"


// Some basic element types
const char * const Element::PUSH_TO_PULL = "h/l";
const char * const Element::PULL_TO_PUSH = "l/h";

// A basic flow code
const char * const Element::COMPLETE_FLOW = "x/x";

// For use in default-logger sequencing
uint64_t
Element::seq=0;

int
Element::elementsLive = 0;

int
Element::elementCounter = 0;

static inline string mk_id_str(long id)
{
  ostringstream s; 
  s << id;
  return s.str();
}

Element::Element(string instanceName) :
  _ninputs(1),
  _noutputs(1),
  _ID(elementCounter++),
  _name(instanceName),
  _logger(NULL),
  _IDstr(mk_id_str(_ID)),
  _state(Element::INACTIVE)
{
  commonConstruction();
}

Element::Element(string instanceName, unsigned ninputs, unsigned noutputs) :
  _ninputs(ninputs),
  _noutputs(noutputs),
  _ID(elementCounter++),
  _name(instanceName),
  _logger(NULL),
  _IDstr(mk_id_str(_ID)),
  _state(Element::INACTIVE)
{
  commonConstruction();
}

Element::~Element()
{
  elementsLive--;
}

void
Element::commonConstruction()
{
  portSetup();
  elementsLive++;
}

// INPUTS AND OUTPUTS

void
Element::portSetup()
{
  // exit on bad counts, or if already initialized
  if (_ninputs < 0 || _noutputs < 0) {
    return;
  }
  
  // Enlarge port arrays if necessary
  _inputs.resize(_ninputs);
  for (unsigned i = 0;
       i < _ninputs;
       i++) {
    _inputs[i].reset(new Port());
  }

  _outputs.resize(_noutputs);
  for (unsigned i = 0;
       i < _noutputs;
       i++) {
    _outputs[i].reset(new Port());
  }
}


bool
Element::ports_frozen() const
{
  assert(0);                    // Deal with freezing without port0
  return false;
}

unsigned Element::addInputPort() 
{
  for (unsigned port=0; port < _inputs.size(); port++) {
    if (_inputs[port] == 0) {
      _inputs[port].reset(new Port());
      _ninputs++;
      return port;
    }
  }
  _inputs.push_back(PortPtr(new Port())); 
  _ninputs++;
  assert (_ninputs == _inputs.size());
  return _ninputs - 1;	// Return the last port number
}

unsigned Element::addOutputPort()
{
  for (unsigned port=0; port < _outputs.size(); port++) {
    if (_outputs[port] == 0) {
      _outputs[port].reset(new Port());
      _noutputs++;
      return port;
    }
  }
  _outputs.push_back(PortPtr(new Port())); 
  _noutputs++;
  assert (_noutputs == _outputs.size());
  return _noutputs - 1;	// Return the last port number
}

int Element::deleteInputPort(unsigned port)
{
  if (port < _inputs.size()) {
    _inputs[port].reset();
    _ninputs--;
    return port;
  }
  return -1;
}

int Element::deleteOutputPort(unsigned port)
{
  if (port < _outputs.size()) {
    _outputs[port].reset();
    _noutputs--;
    return port;
  }
  return -1;
}

int Element::connect_input(unsigned i, Element *f, unsigned port)
{
  if (i >= 0 && i < _inputs.size()) {
    _inputs[i].reset(new Port(this, f, port));
    return 0;
  } else
    return -1;
}

int Element::connect_output(unsigned o, Element *f, unsigned port)
{
  if (o >= 0 && o < _outputs.size()) {
    _outputs[o].reset(new Port(this, f, port));
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

// RUNNING

int Element::push(int port, TuplePtr p, b_cbv cb)
{
  assert(p != 0);

  // Apply the action
  TuplePtr result = simple_action(p);

  // Did we get a result?
  if (result == 0 || result->size() == 0) {
    // No result
    log(Reporting::WARN,
        -1,
        "push: Input tuple yielded no output tuple");
    return 1;
  } else {
    return output(0)->push(result, cb);
  }
}

TuplePtr Element::pull(int port, b_cbv cb)
{
  while (1) {
    TuplePtr p = input(0)->pull(cb);
    if (p) {
      TuplePtr result = simple_action(p);
      if (result != 0) {
        return result;
      } else {
        // This input yielded no result. Try again.
        log(Reporting::WARN,
            -1,
            "pull: Input tuple yielded no output tuple");
      }
    } else {
      // Didn't get any tuples from my input. Fail.
      return TuplePtr();
    }
  }
}

TuplePtr Element::simple_action(TuplePtr p)
{
  return p;
}

REMOVABLE_INLINE const Element::PortPtr Element::input(unsigned i) const
{
  assert(i < _inputs.size());
  return _inputs[i];
}

REMOVABLE_INLINE const Element::PortPtr Element::output(unsigned o) const
{
  assert(o < _outputs.size());
  return _outputs[o];
}

////////////////////////////////////////////////////////////
// Element::Port


REMOVABLE_INLINE int Element::Port::push(TuplePtr p, b_cbv cb) const
{
  // If I am not connected, I shouldn't be pushed.
  assert(_e);
  return _e->input(_port)->push_incoming(_port, p, cb);
}

REMOVABLE_INLINE TuplePtr Element::Port::pull(b_cbv cb) const
{
  // If I am not connected, I shouldn't be pulled.
  assert(_e);
  TuplePtr p = _e->output(_port)->pull_outgoing(_port, cb);
  return p;
}

REMOVABLE_INLINE int Element::Port::push_incoming(int port, TuplePtr p, b_cbv cb) const
{
  return _owner->push(port, p, cb);
}

REMOVABLE_INLINE TuplePtr Element::Port::pull_outgoing(int port, b_cbv cb ) const
{
  TuplePtr t = _owner->pull(port, cb);
  return t;
}


/** Construct a detached free port */
REMOVABLE_INLINE Element::Port::Port() :
  _e(0),
  _port(NOT_INITIALIZED),
  _cb(0)
{
}

/** Construct an attached port */
REMOVABLE_INLINE Element::Port::Port(Element *owner,
                                     Element *e,
                                     int p)
  : _e(e),
    _port(p),
    _cb(0),
    _owner(owner)
{
}

int Element::initialize()
{
  return 0;
}

REMOVABLE_INLINE void Element::log(Reporting::Level severity,
                                   int errnum,
                                   string explanation)
{
  // Even this is a shortcut, cut off the process here as well, since
  // creating the instance name is expensive
  if (severity >= Reporting::level()) {
    ostringstream n;
    n << _name << ":" << _IDstr;
    log(n.str(), severity, errnum, explanation);
  }
}

REMOVABLE_INLINE void Element::log(string instanceName,
                                   Reporting::Level severity,
                                   int errnum,
                                   string explanation)
{
  // Check logging level first
  if (severity >= Reporting::level()) {
    if (_logger) {
      _logger->log(class_name(),
                   instanceName,
                   severity,
                   errnum,
                   explanation); 
    } else {
      logDefault(instanceName, severity, errnum, explanation);
    }
  }
}

REMOVABLE_INLINE void Element::logDefault(string instanceName,
                                          Reporting::Level severity,
                                          int errnum,
                                          string explanation)
{
  boost::posix_time::ptime now;
  getTime(now);
  
  TuplePtr t = Tuple::mk();
  t->append(Val_Time::mk(now));
  t->append(Val_UInt64::mk(seq++));
  t->append(Val_Str::mk(class_name()));
  t->append(Val_Str::mk(instanceName));
  t->append(Val_Int32::mk(severity));
  t->append(Val_Int32::mk(errnum));
  t->append(Val_Str::mk(explanation));
  t->freeze();
  TELL_WARN << "PRE-init LOGGER: " << t->toString() << "\n";
}
                            

void
Element::toDot(std::ostream* ostr)
{
  *ostr << ID()                 // unique element ID
        << " [ label=\"{";
  
  // Now figure out how many input ports
  if (ninputs() > 0) {
    *ostr << "{<i0> 0";
    for (unsigned p = 1;
         p < ninputs();
         p++) {
      *ostr << "| <i" << p << "> " << p << " ";
    }
    *ostr << "}|";
  }
      
  // Show the name
  *ostr << class_name() // the official type
        << "\\n"
        << name();   // the official name
  
  // And figure out the output ports.
  if (noutputs() > 0) {
    *ostr << "|{<o0> 0";
    for (unsigned p = 1;
         p < noutputs();
         p++) {
      *ostr << "| <o" << p << "> " << p << " ";
    }
    *ostr << "}";
  }
  
  // Close the element label
  *ostr << "}\" ];\n";
}
