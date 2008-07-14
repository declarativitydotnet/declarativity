// -*- c-basic-offset: 2; related-file-name: "element.C" -*-
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
 * DESCRIPTION: Base class for P2 elements.  Interface is reminiscent of
 * Click's.
 *
 * An element may be in one of two states: CONFIGURATION and RUNNING.
 * During configuration, the element maintains data about its ports
 * (including personalities, flow codes, etc.) that faciliate with
 * static analysis of the big picture of the forwarding engine.  Since
 * all those data aare not useful once the element is up and running, we
 * only keep them during configuration and discard them thereafter.
 *
 * We've also enhanced to push/pull tuple to prevent blocking.
 *
 * Element signatures (flow and personality) are similar to Click's
 * equivalent descriptors.
 *
 */

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include <string>
#include <sstream>
#include <boost/function.hpp>
#include <boost/any.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "inlines.h"
#include "tuple.h"
#include "reporting.h"
#include "loggerI.h"
#include "val_time.h"

#define ELEM_LOG(_sev,_rest)                                     \
  "Element, "                                                    \
  << Val_Time::mk(boost::posix_time::second_clock::local_time())->toString() \
    << ", "                                                      \
       << class_name()                                           \
    << ", "                                                      \
       << name()                                                 \
    << ", "                                                      \
       << _sev                                                   \
          << ", "                                                \
             << _rest

#define ELEM_INFO(_rest) TELL_INFO    \
  << ELEM_LOG(Reporting::INFO, _rest) \
    << "\n"

#define ELEM_WORDY(_rest) TELL_WORDY   \
  << ELEM_LOG(Reporting::WORDY, _rest) \
    << "\n"

#define ELEM_WARN(_rest) TELL_WARN    \
  << ELEM_LOG(Reporting::WARN, _rest) \
    << "\n"

#define ELEM_ERROR(_rest) TELL_ERROR   \
  << ELEM_LOG(Reporting::ERROR, _rest) \
    << "\n"

#define ELEM_OUTPUT(_rest) TELL_OUTPUT  \
  << ELEM_LOG(Reporting::OUTPUT, _rest) \
    << "\n"

using std::string;
using std::ostringstream;

class Element;
/** A handy dandy pointer to elements */
typedef boost::shared_ptr< Element > ElementPtr;

class Plumber;
typedef boost::shared_ptr< Plumber > PlumberPtr;
////////////////////////////////////////////////////////////
// Callbacks
////////////////////////////////////////////////////////////

typedef boost::function<void (void)>        b_cbv;
typedef boost::function<void (int)>         b_cbi;
typedef boost::function<void (std::string)> b_cbs;
typedef boost::function<void (bool)>        b_cbb;

class Element { 
private:
  /** My common constructor method */
  void
  commonConstruction();
  
  /** Shortcut to setting and sanity-checking port counts. */
  void
  portSetup();

public:
  /** The possible states of an element. 
    * An element first becomes ACTIVE after a initialize() call. */
  enum State {INACTIVE=0, ACTIVE}; 

  /**
   * Thrown when an element operation is not supported
   */ 
  class Exception {
  public:
    Exception(string d) : desc_(d) {};
    string toString() { return desc_; };
  private:
    string desc_;
  };

  // Two shorthand processing signatures.  
  static const char * const PUSH_TO_PULL;
  static const char * const PULL_TO_PUSH;

  // The three processing types
  enum Processing { AGNOSTIC = 'a',
                    PUSH = 'h',
                    PULL = 'l' };

  /** The flow specification of the element.  This is a simplified
      version of that used in Click.  A flow spec has the format
      x1x2x3.../y1y2y3... mapping a single character to each input port
      (before the slash) and to each output port (after the slash).  Any
      matching x's and y's indicate that when the associated (agnostic)
      port is bound to a personality (push or pull) then all other ports
      matching the same character will have to be instantiated to the
      same personality.  For example, if the flow code is ab/ab, then
      the first input port must have the same personality as the first
      output port, and the second input port must have the same
      personality as the second output port.  Similarly, ab/aa means
      that the first input port and both output ports must have the same
      personality. Extra characters in a string are ignored.  If there
      are too few characters in a string, the last character is assumed
      to repeat for any missing characters. A dash indicates no flow
      association; matching dashes mean nothing, other than that
      associated ports have no flow association. */
  static const char * const COMPLETE_FLOW;

  /** Indicates in this position there is no flow association between
      inputs and outputs */
  static const char NO_FLOW_ASSOCIATION = '-';
  
  /** Will be defined later */
  class Port;
  
  /** Create a 1-1 element with just its name */
  Element(string instanceName);

  /** Create an element of arbitrary inputs/outputs */
  Element(string instanceName, unsigned ninputs, unsigned noutputs);

  virtual ~Element();

  /** How many elements are currently live? */
  static int elementsLive;

  /** Counter of elements ever created, to be used for IDs */
  static int elementCounter;

  //
  // RUNTIME
  //

  /** If push returns '1', it's OK to send more tuples, and the callback
      has not been registered.  If '0', it's NOT OK to send more tuples,
      and the callback will be invoked as soon as it is. */
  virtual int push(int port, TuplePtr, b_cbv cb);

  /** If pull returns a Tuple, the callback has not been registered and
      there _might_ be another Tuple available.  If it returns null,
      there wasn't another Tuple, and the callback will be invoked when
      there is another one. */
  virtual TuplePtr pull(int port, b_cbv cb);

  /** A simple action for 1-1 elements. If the result is 0, then no
      tuple was produced for push or pull */
  virtual TuplePtr simple_action(TuplePtr p);

  // CHARACTERISTICS
  virtual const char *class_name() const = 0;

  /** The unique ID of the element */
  int ID() const				{ return _ID; }

  /** A descriptive name for the element */
  REMOVABLE_INLINE string
  name() const;

  virtual boost::any getProxy(){ return boost::any();};

  /** Output my Dot description. */
  virtual void
  toDot(std::ostream*);
  
  // INPUTS AND OUTPUTS
  
  // Getters
  unsigned ninputs() const			{ return _ninputs; }
  unsigned noutputs() const			{ return _noutputs; }

  bool ports_frozen() const;

  // CONFIGURATION
  /** Static port connection */
  virtual int connect_input(unsigned i, Element *f, unsigned port);
  virtual int connect_output(unsigned o, Element *f, unsigned port);

  // Called by the plumber before running 
  virtual int initialize();

  /** Set and get the state of this element to s 
    * (caller == Plumber) */
  virtual void state(State s) { _state = s;    }
  virtual State state() const { return _state; }
  
  // PROCESSING, FLOW, AND FLAGS
  virtual const char *processing() const;
  virtual const char *flow_code() const;
  virtual const char *flags() const;



  /** A nested class encapsulating connection stubs into and out of an
      element. */
  class Port { 
  public:
    
    Port();

    ~Port() {};

    Port(Element * owner,
         Element * correspondent,
         int correspondentPortNumber);
    
    operator bool() const		{ return _e != 0; }
    bool initialized() const		{ return _port >= -1; }
    

    Element *element() const		{ return _e; }
    
    /* The port number error values */
    enum PortErrors { NOT_CONNECTABLE = -1,
                      NOT_INITIALIZED = -2 };


    /** The port number at which I am connected at the destination
        element.  If I am not a connectable port (i.e., a pull output
        port or a push input port) then this is NOT_CONNECTABLE.
        Uninitialized ports have this value set to NOT_INITIALIZED. */
    int port() const			{ return _port; }
    

    // PORT ACTIVITY

    /** If push returns '1', it's OK to send more tuples, and the
        callback has not been registered.  If '0', it's NOT OK to send
        more tuples, and the callback will be invoked as soon as it
        is.  */
    int push(TuplePtr p, b_cbv cb) const;

    /** If pull returns a Tuple, the callback has not been registered
        and there _might_ be another Tuple available.  If it returns
        null, there wasn't another Tuple, and the callback will be
        invoked when there is another one. */
    TuplePtr pull(b_cbv cb) const;

    /** A push is called on the input port of an element **/
    int push_incoming(int port, TuplePtr p, b_cbv cb) const;
    
    /** A pull is called on the output port of an element **/
    TuplePtr pull_outgoing(int port, b_cbv cb) const;

  private:
    
    /** With whom am I connecting my owner element? */
    Element *_e;

    /** The port number at which I am connected at the destination
        element.  If I am not a connectable port (i.e., a pull output
        port or a push input port) then this is NOT_CONNECTABLE.
        Uninitialized ports have this value set to NOT_INITIALIZED. */
    int _port;

    /** My callback */
    b_cbv _cb;

    Element *_owner;			// Whose input or output are we?
  };

  typedef boost::shared_ptr< Port > PortPtr;
  typedef std::vector< PortPtr > PortVec;

  const PortPtr input(unsigned) const;
  const PortPtr output(unsigned) const;

  /** Get the port number based on the port key */
  virtual int input(ValuePtr key);
  virtual int output(ValuePtr key); 

  /** Dynamic allocation of ports */
  virtual unsigned add_input();
  virtual unsigned add_output();

  /** Dynamic allocation of ports based on port keys */
  virtual unsigned add_input(ValuePtr key);
  virtual unsigned
  add_output(ValuePtr key);

  /** Remove the port indicated by port number */
  virtual int remove_input(unsigned p);
  virtual int remove_output(unsigned p);

  /** Remove the port indicated by port keys 
   *  Return: The port number that was removed
   */
  virtual int remove_input(ValuePtr key);
  virtual int remove_output(ValuePtr key);

  /** My input ports */
  PortVec _inputs;

  /** My output ports */
  PortVec _outputs;

  /** How many inputs do I have? */
  unsigned _ninputs;

  /** How many outputs do I have? */
  unsigned _noutputs;

  /** My ID */
  long _ID;

  Element(const Element &);
  Element &operator=(const Element &);
  
  /** For my default logger's sequencing */
  static uint64_t seq;

  /** My instance name */
  string _name;

  LoggerI*       _logger;

protected:
  unsigned addInputPort();
  unsigned addOutputPort();
  int deleteInputPort(unsigned);
  int deleteOutputPort(unsigned);

private:
  /** My ID in text */
  string _IDstr;

  /** State of this element */
  State _state; 
  
};

#endif /* __ELEMENT_H_ */
