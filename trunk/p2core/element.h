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
 * We've also enhanced to push/pull tuple to prevent blocking (something
 * Click, being a router, doesn't need to bother with). 
 */

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "inlines.h"
#include "tuple.h"

class Router;
typedef ref< Router > RouterRef;
typedef ptr< Router > RouterPtr;


class Element { 
 public:
  
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

  static const char NO_FLOW_ASSOCIATION = '-';

  class Port;
  
  Element();
  Element(int ninputs, int noutputs);
  virtual ~Element();
  static int nelements_allocated;
  static int elementCounter;

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

  // A simple action for 1-1 elements. If the result is 0, then no tuple
  // was produced for push or pull
  virtual TuplePtr simple_action(TupleRef p);

  /** Return true if did useful work */
  virtual bool run_task();

  /** Return true if did useful work. */
  virtual bool run_timer();

  // CHARACTERISTICS
  virtual const char *class_name() const = 0;
  int ID() const				{ return _ID; }

  /** Return the router that contains me */
  RouterPtr router() const			{ return _router; }


  // INPUTS AND OUTPUTS
  int ninputs() const				{ return _ninputs; }
  int noutputs() const				{ return _noutputs; }
  void set_ninputs(int);
  void set_noutputs(int);
  void add_input()				{ set_ninputs(ninputs()+1); }
  void add_output()				{ set_noutputs(noutputs()+1); }
  bool ports_frozen() const;

  // CONFIGURATION
  int connect_input(int i, Element *f, int port);
  int connect_output(int o, Element *f, int port);
  virtual int initialize();
  
  // PROCESSING, FLOW, AND FLAGS
  virtual const char *processing() const;
  virtual const char *flow_code() const;
  virtual const char *flags() const;
  
  // METHODS USED BY `ROUTER'
  
  /** Attach me to a router */
  void attach_router(RouterRef r)		{ _router = r; }









  /** A nested class encapsulating connection stubs into and out of an
      element. */
  class Port { 
  public:

    Port();

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
    int push(TupleRef p, cbv cb) const;

    /** If pull returns a Tuple, the callback has not been registered
        and there _might_ be another Tuple available.  If it returns
        null, there wasn't another Tuple, and the callback will be
        invoked when there is another one. */
    TuplePtr pull(cbv cb) const;

#if P2_STATS >= 1
    unsigned ntuples() const		{ return _tuples; }
#endif

   private:
    
    /** With whom am I connecting my owner element? */
    Element *_e;

    /** The port number at which I am connected at the destination
        element.  If I am not a connectable port (i.e., a pull output
        port or a push input port) then this is NOT_CONNECTABLE.
        Uninitialized ports have this value set to NOT_INITIALIZED. */
    int _port;

    /** My callback */
    cbv _cb;
    
#if P2_STATS >= 1
    mutable unsigned _tuples;		// How many tuples have we moved?
#endif
#if P2_STATS >= 2
    Element *_owner;			// Whose input or output are we?
#endif
  };

  typedef ptr< Port > PortPtr;
  typedef ref< Port > PortRef;
  typedef vec< PortPtr > PortVec;




  const PortRef input(int) const;
  const PortRef output(int) const;








 private:

  /** My router */
  RouterPtr _router;

  /** My input ports */
  PortVec _inputs;

  /** My output ports */
  PortVec _outputs;

  /** How many inputs do I have? */
  int _ninputs;

  /** How many outputs do I have? */
  int _noutputs;

  /** My ID */
  long _ID;

  Element(const Element &);
  Element &operator=(const Element &);
  
  /** Set my ports before I'm initialized */
  void set_nports(int, int);
};


#if P2_STATS >= 2
# define PORT_CTOR_INIT(o) , _tuples(0), _owner(o)
#else
# if P2_STATS >= 1
#  define PORT_CTOR_INIT(o) , _tuples(0)
# else
#  define PORT_CTOR_INIT(o)
# endif
#endif

/** A handy dandy reference to elements */
typedef ref< Element > ElementRef;
typedef ptr< Element > ElementPtr;

#endif /* __ELEMENT_H_ */
