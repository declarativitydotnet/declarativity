// -*- c-basic-offset: 2; related-file-name: "router.C" -*-
/*
 * @(#)$Id$
 * Loosely inspired from the Click Router class, by Eddie Kohler
 * 
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
 * Copyright (c) 2000 Mazu Networks, Inc.
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
 * DESCRIPTION: The router shell providing plumbing functionality.
 */

#ifndef __ROUTER_H__
#define __ROUTER_H__

#include <inlines.h>
#include <element.h>
#include <elementSpec.h>
#include <vec.h>
#include <master.h>
#include <loggerI.h>

// Put here to resolve cyclical dependencies
class Master;
typedef ref< Master > MasterRef;

/** A handy dandy type for router references */
class Router;
typedef ref< Router > RouterRef;
typedef ptr< Router > RouterPtr;


class Router {
public:
  
  /** An auxilliary structure to help pass around element connection
      specifications */
  struct Hookup {
    /** The element from which this hookup originates */
    ElementSpecRef fromElement;
    
    /** The port number at the fromElement */
    int fromPortNumber;

    /**  The element to which this hookup goes */
    ElementSpecRef toElement;

    /** The port number at the toElement */
    int toPortNumber;

    Hookup(ElementSpecRef fe, int fp,
           ElementSpecRef te, int tp)
      : fromElement(fe), fromPortNumber(fp),
        toElement(te), toPortNumber(tp) {};
  };
  typedef ref< Hookup > HookupRef;
  typedef ptr< Hookup > HookupPtr;

  struct Configuration {
    /** The elements */
    vec< ElementSpecRef > elements;

    /** The hookups */
    vec< HookupRef > hookups;

    ElementSpecRef addElement(ref<Element> e) {
      ElementSpecRef r = New refcounted<ElementSpec>(e);
      elements.push_back(r);
      return r;
    }
    void hookUp(ElementSpecRef src, int src_port,
		ElementSpecRef dst, int dst_port ) {
      hookups.push_back(New refcounted<Hookup>(src,src_port, dst,dst_port));
    }

    Configuration() {};
    Configuration(ref< vec< ElementSpecRef > > e,
                  ref< vec< HookupRef > > h)
      : elements(*e), hookups(*h) {};

  };
  typedef ref< Configuration > ConfigurationRef;
  typedef ptr< Configuration > ConfigurationPtr;

  /** Create a new router given a configuration of constructed but not
      necessarily configured elements. */
  Router(ConfigurationRef configuration,
         MasterRef master);

  ~Router();

  // INITIALIZATION
  
  /** Initialize the engine from the configuration */
  int initialize(RouterRef);

  /** Start the router */
  void activate();





  static void static_initialize();
  static void static_cleanup();

  /** Router state */
  enum { ROUTER_NEW,
         ROUTER_PRECONFIGURE,
         ROUTER_PREINITIALIZE,
         ROUTER_LIVE,
         ROUTER_DEAD };

  bool initialized() const			{ return _state == ROUTER_LIVE; }

  // ELEMENTS
  int nelements() const				{ return _elements->size(); }
  const ref< vec< ElementRef > > elements() const { return _elements; }
  
  // MASTER scheduler
  MasterRef master() const			{ return _master; }

  // LOGGING infrastructure
  
  /** The router-wide logger.  The pointer to the logger should not be
      cached or used at any time other than immediately after the
      invocation of this method. */
  REMOVABLE_INLINE LoggerI * logger()		{ return _logger; }

  /** Install a different logger for this router. This returns a pointer
      to the previous logger.  If non zero, it is the caller's
      responsibility to delete that logger if it's on the heap. */
  REMOVABLE_INLINE LoggerI * logger(LoggerI * newLogger);
  
private:
  
  /** The scheduler */
  MasterRef _master;
  
  ref< vec< ElementRef > > _elements;
  
  /** The router state */
  int _state;

  /** The configuration spec of the router */
  ConfigurationPtr _configuration;

  /** Are the configuration hookups refering to existing elements and
      non-negative ports? */
  int check_hookup_elements();

  /** Are the port numbers within the attached element's range? */
  int check_hookup_range();

  /** Is personality semantics correctly applied to hookups?  This only
      checks that the end-points of a hookup are consistent.  Unlike
      Click, it does not follow flow codes through elements. */
  int check_push_and_pull();

  /** Are any ports multiply connected?  Are all ports attached to
      something?  Unlike Click, we require all ports to be attached to
      something (exactly one something), even pull outputs and push
      inputs. */
  int check_hookup_completeness();

  /** Perform the actual hooking up of real elements from the specs. No
      error checking at this point.  */
  void set_connections();

  /** Convenience function for adding a created (but not initialized)
      element into the router. */
  void add_element(RouterRef, ElementRef);

  friend class Master;
  friend class Task;

  /** My local logger */
  LoggerI * _logger;
};

#endif
