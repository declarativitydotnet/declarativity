// -*- c-basic-offset: 2; related-file-name: "router.h" -*-
/*
 * @(#)$Id$
 * Modified from the Click Element base class by Eddie Kohler
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
 * DESCRIPTION: The tuple router class providing support for element
 * plumbing.
 * 
 * Click machinery works as follows:
 *
 *  Lexer finds all distinct elements and their interconnections
 *
 *  For every found element, a default constructor is called and the
 *  element is added into the router (as a singleton object)
 *
 *  For every found connection, the router adds a connection spec
 *  (called a "hookup") from the source element to the destination
 *  element to a queue of new connections to be created.
 *
 *  The router is "initialized."
 *
 *    The queued hookups are checked for sanity (they connect existing
 *    elements and existing port numbers).  Incorrect hookups are
 *    dropped. Any errors in the hookups cause the initialization to
 *    fail.
 *
 *    Elements are sorted by the order in which they wish to be
 *    configured (from 4 distinct phases).  Then they are notified about
 *    when they should expect to be configured and how many of their
 *    ports have clients (i.e., other elements connecting to them).
 *   
 *    In sorted order, every element is configured with its
 *    configuration data (arguments, parameters, etc.).  If any of the
 *    elements failed its configuration, the initialization stops.
 *
 *    After element configuration, all hookups are checked again to
 *    ensure that port numbers used are consistent with the fully
 *    configured elements.
 *
 *    Next, hookups are checked for semantic correctness in terms of
 *    push/pull personality.  According to connectivity, agnostic elements
 *    are assigned a personality, e.g., if a push output port is
 *    connected to an agnostic input port, then the input port is
 *    instantiated as a push port.  The ports are created and
 *    initialized, making appropriate ports not connectable (i.e., pull
 *    outputs or push inputs).
 *
 *    Next, hookups are checked for duplication.  Connectable ports
 *    (i.e., push outputs and pull inputs) can only be connected to a
 *    single counterpart.  Also unused ports are identified.  Neither
 *    condition permits initialization to continue.
 *
 *    Next the actual plumbing occurs.  Actual element objects are
 *    introduced to each other and connected at the appropriate ports.
 *
 *    Once connected, elements are themselves initialized in their
 *    configuration order.  Initialization consists of the creation of
 *    their handlers (introspection interfaces) and any element-specific
 *    other initialization.  This is the stage at which "active"
 *    elements place themselves in the run queue.
 *
 *
 * Synchronization machinery:
 *  
 *  Each router has a runcount used as a semaphore (it's an integer
 *  protected by a global lock stored at the master).
 *
 *  The master also have a runcount which is the minimum of all router
 *  runcount values.
 *
 *  Correctly initialized routers have runcount of at least
 *  1. Incorrectly initialized router have non-positive runcounts.
 *
 *  A router thread runs continuously, as long as the master runcount
 *  remains positive.  When a router thread detects that the master
 *  runcount is no longer positive, it kicks the master to clean up its
 *  routers. If after the cleanup the master reports there's more
 *  running to be had by all, the driver method of the router thread
 *  continues the loop.
 */


#include <router.h>
#include <iostream>
#include <set>

void Router::set_connections()
{
  // actually assign ports
  for (uint i = 0;
       i < _configuration->hookups.size();
       i++) {
    HookupRef hookup = _configuration->hookups[i];
    ElementSpecRef fromElement = hookup->fromElement;
    ElementSpecRef toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;

    fromElement->element()->connect_output(fromPort,
                                           toElement->element(),
                                           toPort);
    toElement->element()->connect_input(toPort,
                                        fromElement->element(),
                                        fromPort);
  }
}



int Router::check_hookup_completeness()
{
  // Check duplicates
  int duplicates = 0;
  for (uint i = 0;
       i < _configuration->hookups.size();
       i++) {
    HookupRef hookup = _configuration->hookups[i];
    
    ElementSpecRef fromElement = hookup->fromElement;
    ElementSpecRef toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;
    
    int dup =
      fromElement->output(fromPort)->counterpart(toElement->element());
    if (dup > 0) {
      std::cerr << "Output port " << fromPort << " of element "
                << fromElement->toString()
                << " reused\n";
    }
    duplicates += dup;
    dup =
      toElement->input(toPort)->counterpart(fromElement->element());
    if (dup > 0) {
      std::cerr << "Input port " << toPort << " of element "
                << toElement->toString()
                << " reused\n";
    }
    duplicates += dup;
  }

  // Check unuseds
  int unuseds = 0;
  for (uint i = 0;
       i < _configuration->elements.size();
       i++) {
    ElementSpecRef element = (_configuration->elements)[i];
    for (int in = 0;
         in < element->element()->ninputs();
         in++) {
      if (element->input(in)->counterpart() == 0) {
        unuseds++;
        std::cerr << "Input port " << in << " of element "
                  << element->toString()
                  << " unused\n";
      }
    }
    for (int out = 0;
         out < element->element()->noutputs();
         out++) {
      if (element->output(out)->counterpart() == 0) {
        unuseds++;
        std::cerr << "Output port " << out << " of element "
                  << element->toString()
                  << " unused\n";
      }
    }
  }
  
  if ((unuseds > 0) ||
      (duplicates > 0)) {
    return -1;
  } else {
    return 0;
  }
}



int Router::check_push_and_pull()
{
  int errors = 0;
  // For every hookup...
  ElementSpec::UnificationResult result;
  while (1) {
    bool progress = false;

    for (uint i = 0;
         i < _configuration->hookups.size();
         i++) {
      HookupRef hookup = (_configuration->hookups)[i];
      
      ElementSpecRef fromElement = hookup->fromElement;
      ElementSpecRef toElement = hookup->toElement;
      int fromPort = hookup->fromPortNumber;
      int toPort = hookup->toPortNumber;
      // By now, the ports should be acceptable
      assert((fromPort < fromElement->element()->noutputs()) &&
             (toPort < toElement->element()->ninputs()));
      
      switch (fromElement->output(fromPort)->personality()) {
      case Element::PUSH:
        // If from port is push...
        switch (toElement->input(toPort)->personality()) {
        case Element::PUSH:
          // If to port is push, we're ok
          break;
          
        case Element::PULL:
          // If to port is pull, we're not OK
          std::cerr << "Hookup from PUSH to PULL found\n";
          errors++;
          break;
          
        case Element::AGNOSTIC:
          // If to port is agnostic, make it push and unify it
          toElement->input(toPort)->personality(Element::PUSH);
          ElementSpec::UnificationResult result =
            toElement->unifyInput(toPort);
          if (result == ElementSpec::CONFLICT) {
            errors++;
          };
          progress = true;
          break;
        }
        break;
        
      case Element::PULL:
        // Else, if from port is pull...
        switch (toElement->input(toPort)->personality()) {
        case Element::PUSH:
          // If to port is push, we're not Ok
          std::cerr << "Hookup from PULL to PUSH found\n";
          errors++;
          break;
          
        case Element::PULL:
          // If to port is pull, we're ok
          break;
          
        case Element::AGNOSTIC:
          // If to port is agnostic, make it pull
          toElement->input(toPort)->personality(Element::PULL);
          ElementSpec::UnificationResult result =
            toElement->unifyInput(toPort);
          if (result == ElementSpec::CONFLICT) {
            errors++;
          };
          progress = true;
          break;
        }
        break;
        
      case Element::AGNOSTIC:
        // Else, if from port is agnostic...
        switch (toElement->input(toPort)->personality()) {
        case Element::PUSH:
          // If to port is push, make from push
          fromElement->output(fromPort)->personality(Element::PUSH);
          result = fromElement->unifyOutput(fromPort);
          if (result == ElementSpec::CONFLICT) {
            errors++;
          };
          progress = true;
          break;
          
        case Element::PULL:
          // If to port is pull, make from pull
          fromElement->output(fromPort)->personality(Element::PULL);
          result = fromElement->unifyOutput(fromPort);
          if (result == ElementSpec::CONFLICT) {
            errors++;
          };
          progress = true;
          break;
          
        case Element::AGNOSTIC:
          // If to port is agnostic, we're ok
          break;
        }
      }
    }
    
    if (!progress) {
      break;
    }
  } 

  // Original Click flow code checker does the following:
  //
  // For every agnostic input port
  //   Find all output ports to which the input port is linked with a
  //     flow code (see below how this is done).
  //   For every such output port
  //     Create a false hookup from the input port to the output port
  // Forever
  //   For every hookup
  //     Check semantic consistency and push unifications
  //   If no changes made, exit loop
  // 
  // For a given agnostic input port, the original Click flow code
  // finds all associated output ports as follows:
  // 
  // Find the flow code for the input port
  // Set all flow codes that match that of the input port
  // For all output ports
  //   Find the appropriate flow code
  //   Set all flow codes that match that of the output port
  //   If the output code matches have an intersection with the input
  //     Set the output port as a linked port to the input
  //
  // In the P2 implementation, flow codes are much simpler (single
  // character only)

  if (errors > 0) {
    return -1;
  } else {
    return 0;
  }
}


int Router::check_hookup_range()
{
  // Check each hookup to ensure its port numbers are within range
  int errors = 0;
  for (uint i = 0;
       i < _configuration->hookups.size();
       i++) {
    HookupRef hookup = (_configuration->hookups)[i];
    
    if (hookup->fromPortNumber >= hookup->fromElement->
        element()->noutputs()) {
      std::cerr << "Cannot connect from port " <<
        hookup->fromPortNumber << " in element " <<
        hookup->fromElement->toString() << "\n";
      errors++;
    }
    if (hookup->toPortNumber >= hookup->toElement->
        element()->ninputs()) {
      std::cerr << "Cannot connect to port " <<
        hookup->toPortNumber << " in element " <<
        hookup->toElement->toString() << "\n";
      errors++;
    }
  }

  if (errors > 0) {
    return -1;
  } else {
    return 0;
  }
}


Router::Router(ConfigurationRef c,
               MasterRef m)
  : _master(m),
    _elements(New refcounted< vec< ElementRef > >()),
    _state(ROUTER_NEW),
    _configuration(c)
{
}

Router::~Router()
{
  // Unschedule if running

  // Uninitialize elements in reverse order

  // Kill elements
}



/**
 * initialize
 *
 * This performs a simplified (compared to Click) initialization of the
 * element topology:
 *
 * - Check that connections refer to correct elements
 *
 * - Skip configuration ordering, since for now we start with
 * preconfigured elements.
 *
 * The router ref parameter is not stored locally (this could prevent
 * routers from being freed in the end) but used to initialize elements
 * down the line.
 *
 */
int Router::initialize(RouterRef myRef)
{
  // Am I already initialized?
  if (_state != ROUTER_NEW) {
    std::cerr << "Second attempt to initialize router";
    return -1;
  }
  _state = ROUTER_PRECONFIGURE;
  assert(_configuration != 0);

  // Are the hookups pointing to existing elements and ports?
  if (check_hookup_elements() < 0) {
    return -1;
  }

  // Are the port numbers plausible?
  if (check_hookup_range() < 0) {
    return -1;
  }

  // Check push/pull semantics
  if (check_push_and_pull() < 0) {
    return -1;
  }

  // Check hookup completeness.  All ports have something attached to
  // them
  if (check_hookup_completeness() < 0) {
    return -1;
  }

  // Time to do the actual hooking up.  Create element connections. Move
  // the elements from the configuration to the router. Trash the
  // configuration.
  set_connections();
  for (uint i = 0;
       i < _configuration->elements.size();
       i++) {
    ElementRef theElement = (_configuration->elements)[i]->element();
    add_element(myRef,
                theElement);

    // Initialize the element
    theElement->initialize();
  }
  _configuration = 0;
  _state = ROUTER_LIVE;
  
  return 0;
}

void Router::add_element(RouterRef myRef,
                         ElementRef e)
{
  // router now owns the element
  _elements->push_back(e);
  e->attach_router(myRef);
}


int Router::check_hookup_elements()
{
  // Put all (real not spec) elements in a set to be searchable
  std::set< ElementRef > elementSet;
  for (uint i = 0;
       i < _configuration->elements.size();
       i++) {
    elementSet.insert((_configuration->elements)[i]->element());
  }
  
  // Check each hookup to ensure it connects valid element references
  int errors = 0;
  for (uint i = 0;
       i < _configuration->hookups.size();
       i++) {
    HookupRef hookup = (_configuration->hookups)[i];
    std::set< ElementRef >::iterator found =
      elementSet.find(hookup->fromElement->element());
    if ((found == elementSet.end()) ||
        (*found != hookup->fromElement->element())) {
      // This hookup comes from a non-existing element 
      std::cerr << "Non-existent from element " <<
        hookup->fromElement->toString() << "\n";
      errors++;
    }
    found = elementSet.find(hookup->toElement->element());
    if ((found == elementSet.end()) ||
        (*found != hookup->toElement->element())) {
      // This hookup goes to a non-existing element 
      std::cerr << "Non-existent to element " <<
        hookup->toElement->toString() << "\n";
      errors++;
    }
    if (hookup->fromPortNumber < 0) {
      // Negative port is bad
      std::cerr << "Bad hookup from port " << hookup->fromPortNumber << "\n";
      errors++;
    }
    if (hookup->toPortNumber < 0) {
      // Negative port is bad
      std::cerr << "Bad hookup to port " << hookup->toPortNumber << "\n";
      errors++;
    }
  }
    
  if (errors > 0) {
    // Ooops, there were problems
    return -1;
  } else {
    return 0;
  }
}





void Router::activate()
{
  if (_state != ROUTER_LIVE) {
    return;
  }
}



























