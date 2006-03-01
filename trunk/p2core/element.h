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
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
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
 * Click, being a plumber, doesn't need to bother with). 
 */

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "inlines.h"
#include "tuple.h"
#include "loggerI.h"
#include "loop.h"
  
#define ELEM_LOG(_sev,_errnum,_rest) do { ostringstream _sb; _sb << _rest; log(_sev,_errnum,_sb.str()); } while (false)
#define ELEM_INFO(_rest) ELEM_LOG(LoggerI::INFO, 0, _rest)


class Plumber;
typedef boost::shared_ptr< Plumber > PlumberPtr;


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
  
  Element(string instanceName);
  Element(string instanceName, int ninputs, int noutputs);
  virtual ~Element();
  static int nelements_allocated;
  static int elementCounter;

  //
  // RUNTIME
  //

  // If push returns '1', it's OK to send more tuples, and the
  // callback has not been registered.  If '0', it's NOT OK to send
  // more tuples, and the callback will be invoked as soon as it is. 
  virtual int push(int port, TuplePtr, b_cbv cb);

  // If pull returns a Tuple, the callback has not been registered and
  // there _might_ be another Tuple available.  If it returns null,
  // there wasn't another Tuple, and the callback will be invoked when
  // there is another one. 
  virtual TuplePtr pull(int port, b_cbv cb);

  // A simple action for 1-1 elements. If the result is 0, then no tuple
  // was produced for push or pull
  virtual TuplePtr simple_action(TuplePtr p);

  // CHARACTERISTICS
  virtual const char *class_name() const = 0;
  int ID() const				{ return _ID; }
  string name() const				{ return _name; }

  /** Return the plumber that contains me */
  PlumberPtr plumber() const			{ return _plumber; }


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

  // LOGGING facilities

  /** Log something to the default element logger */
  REMOVABLE_INLINE void log(string instanceName,
                            LoggerI::Level severity,
                            int errnum,
                            string explanation);

  /** Call the log method without an instance name.  Use the element ID
      instead. */
  REMOVABLE_INLINE void log(LoggerI::Level severity,
                            int errnum,
                            string explanation);
  
  /** Call the default logger, if the plumber's logger is unavailable */
  REMOVABLE_INLINE void logDefault(string instanceName,
                                   LoggerI::Level severity,
                                   int errnum,
                                   string explanation);

  // METHODS USED BY `PLUMBER'
  
  /** Attach me to a plumber */
  void attach_plumber(PlumberPtr r)		{ _plumber = r; }

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


  const PortPtr input(int) const;
  const PortPtr output(int) const;

  /** My plumber */
  PlumberPtr _plumber;

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

  /** For my default logger's sequencing */
  static uint64_t seq;

  /** My instance name */
  string _name;

protected:

  /** My ID in text */
  string _IDstr;

};

/** A handy dandy pointer to elements */
typedef boost::shared_ptr< Element > ElementPtr;

#endif /* __ELEMENT_H_ */
