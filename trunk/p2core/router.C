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
  for (int i = 0;
       i < _configuration->hookups->size();
       i++) {
    HookupRef hookup = (*_configuration->hookups)[i];
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
       i < _configuration->hookups->size();
       i++) {
    HookupRef hookup = (*_configuration->hookups)[i];
    
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
  for (int i = 0;
       i < _configuration->elements->size();
       i++) {
    ElementSpecRef element = (*_configuration->elements)[i];
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
         i < _configuration->hookups->size();
         i++) {
      HookupRef hookup = (*_configuration->hookups)[i];
      
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
       i < _configuration->hookups->size();
       i++) {
    HookupRef hookup = (*_configuration->hookups)[i];
    
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
 *
 */
int Router::initialize()
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
  for (int i = 0;
       i < _configuration->elements->size();
       i++) {
    ElementRef theElement = (*_configuration->elements)[i]->element();
    add_element(theElement);

    // Initialize the element
    theElement->initialize();
  }
  _configuration = 0;
  _state = ROUTER_LIVE;
  
  return 0;
}

void Router::add_element(ElementRef e)
{
  // router now owns the element
  _elements->push_back(e);
  e->attach_router(this);
}


int Router::check_hookup_elements()
{
  // Put all (real not spec) elements in a set to be searchable
  std::set< ElementRef > elementSet;
  ref< vec< ElementSpecRef > > elements =
    _configuration->elements;
  for (uint i = 0;
       i < _configuration->elements->size();
       i++) {
    elementSet.insert((*_configuration->elements)[i]->element());
  }
  
  // Check each hookup to ensure it connects valid element references
  int errors = 0;
  for (uint i = 0;
       i < _configuration->hookups->size();
       i++) {
    HookupRef hookup = (*_configuration->hookups)[i];
    if (*elementSet.find(hookup->fromElement->element()) !=
        hookup->fromElement->element()) {
      // This hookup comes from a non-existing element 
      std::cerr << "Non-existent from element " <<
        hookup->fromElement->toString() << "\n";
      errors++;
    }
    if (*elementSet.find(hookup->toElement->element()) !=
        hookup->toElement->element()) {
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



























#if 0


// HANDLERS

String
Router::Handler::unparse_name(Element *e, const String &hname)
{
  if (e && e != e->router()->root_element())
    return e->id() + "." + hname;
  else
    return hname;
}

String
Router::Handler::unparse_name(Element *e) const
{
  return unparse_name(e, _name);
}


// Private functions for finding and storing handlers

// 11.Jul.2000 - We had problems with handlers for medium-sized configurations
// (~400 elements): the Linux kernel would crash with a "kmalloc too large".
// The solution: Observe that most handlers are shared. For example, all
// 'name' handlers can share a Handler structure, since they share the same
// read function, read thunk, write function (0), write thunk, and name. This
// introduced a bunch of structure to go from elements to handler indices and
// from handler names to handler indices, but it was worth it: it reduced the
// amount of space required by a normal set of Router::Handlers by about a
// factor of 100 -- there used to be 2998 Router::Handlers, now there are 30.
// (Some of this space is still not available for system use -- it gets used
// up by the indexing structures, particularly _ehandlers. Every element has
// its own list of "element handlers", even though most elements with element
// class C could share one such list. The space cost is about (48 bytes * # of
// elements) more or less. Detecting this sharing would be harder to
// implement.)

int
Router::find_ehandler(int eindex, const String &name) const
{
  int eh = _ehandler_first_by_element[eindex];
  int star_h = -1;
  while (eh >= 0) {
    int h = _ehandler_to_handler[eh];
    const String &hname = _handlers[h].name();
    if (hname == name)
      return eh;
    else if (hname.length() == 1 && hname[0] == '*')
      star_h = h;
    eh = _ehandler_next[eh];
  }
  if (_allow_star_handler && star_h >= 0 && _handlers[star_h].writable()) {
    _allow_star_handler = false;
    if (_handlers[star_h].call_write(name, element(eindex), ErrorHandler::default_handler()) >= 0)
      eh = find_ehandler(eindex, name);
    _allow_star_handler = true;
  }
  return eh;
}

REMOVABLE_INLINE Router::Handler
Router::fetch_handler(const Element *e, const String &name)
{
  if (const Handler *h = handler(e, name))
    return *h;
  else
    return Handler(name);
}

void
Router::store_local_handler(int eindex, const Handler &to_store)
{
  bool allow_star_handler = _allow_star_handler;
  _allow_star_handler = false;
  int old_eh = find_ehandler(eindex, to_store.name());
  _allow_star_handler = allow_star_handler;
  if (old_eh >= 0)
    _handlers[_ehandler_to_handler[old_eh]]._use_count--;
  
  // find the offset in _name_handlers
  int name_index;
  for (name_index = 0;
       name_index < _handler_first_by_name.size();
       name_index++) {
    int h = _handler_first_by_name[name_index];
    if (_handlers[h]._name == to_store._name)
      break;
  }
  if (name_index == _handler_first_by_name.size())
    _handler_first_by_name.push_back(-1);

  // find a similar handler, if any exists
  int h = _handler_first_by_name[name_index];
  int blank_h = -1;
  int stored_h = -1;
  while (h >= 0 && stored_h < 0) {
    const Handler &han = _handlers[h];
    if (han.compatible(to_store))
      stored_h = h;
    else if (han._use_count == 0)
      blank_h = h;
    h = han._next_by_name;
  }

  // if none exists, assign this one to a blank spot
  if (stored_h < 0 && blank_h >= 0) {
    stored_h = blank_h;
    _handlers[stored_h] = to_store;
    _handlers[stored_h]._use_count = 0;
  }

  // if no blank spot, add a handler
  if (stored_h < 0) {
    if (_nhandlers >= _handlers_cap) {
      int new_cap = (_handlers_cap ? 2*_handlers_cap : 16);
      Handler *new_handlers = new Handler[new_cap];
      if (!new_handlers) {	// out of memory
	if (old_eh >= 0)	// restore use count
	  _handlers[_ehandler_to_handler[old_eh]]._use_count++;
	return;
      }
      for (int i = 0; i < _nhandlers; i++)
	new_handlers[i] = _handlers[i];
      delete[] _handlers;
      _handlers = new_handlers;
      _handlers_cap = new_cap;
    }
    stored_h = _nhandlers;
    _nhandlers++;
    _handlers[stored_h] = to_store;
    _handlers[stored_h]._use_count = 0;
    _handlers[stored_h]._next_by_name = _handler_first_by_name[name_index];
    _handler_first_by_name[name_index] = stored_h;
  }

  // point ehandler list at new handler
  if (old_eh >= 0)
    _ehandler_to_handler[old_eh] = stored_h;
  else {
    int new_eh = _ehandler_to_handler.size();
    _ehandler_to_handler.push_back(stored_h);
    _ehandler_next.push_back(_ehandler_first_by_element[eindex]);
    _ehandler_first_by_element[eindex] = new_eh;
  }

  // increment use count
  _handlers[stored_h]._use_count++;
}

void
Router::store_global_handler(const Handler &h)
{
  for (int i = 0; i < nglobalh; i++)
    if (globalh[i]._name == h._name) {
      globalh[i] = h;
      globalh[i]._use_count = 1;
      return;
    }
  
  if (nglobalh >= globalh_cap) {
    int n = (globalh_cap ? 2 * globalh_cap : 4);
    Router::Handler *hs = new Router::Handler[n];
    if (!hs)			// out of memory
      return;
    for (int i = 0; i < nglobalh; i++)
      hs[i] = globalh[i];
    delete[] globalh;
    globalh = hs;
    globalh_cap = n;
  }

  globalh[nglobalh] = h;
  globalh[nglobalh]._use_count = 1;
  nglobalh++;
}

REMOVABLE_INLINE void
Router::store_handler(const Element *e, const Handler &to_store)
{
  if (e)
    e->router()->store_local_handler(e->eindex(), to_store);
  else
    store_global_handler(to_store);
}


// Public functions for finding handlers

const Router::Handler *
Router::handler(const Router *r, int hi)
{
  if (r && hi >= 0 && hi < r->_nhandlers)
    return &r->_handlers[hi];
  else if (hi >= FIRST_GLOBAL_HANDLER && hi < FIRST_GLOBAL_HANDLER + nglobalh)
    return &globalh[hi - FIRST_GLOBAL_HANDLER];
  else
    return 0;
}

const Router::Handler *
Router::handler(const Element *e, const String &hname)
{
  if (e && e != e->router()->_root_element) {
    const Router *r = e->router();
    int eh = r->find_ehandler(e->eindex(), hname);
    if (eh >= 0)
      return &r->_handlers[r->_ehandler_to_handler[eh]];
  } else {			// global handler
    for (int i = 0; i < nglobalh; i++)
      if (globalh[i]._name == hname)
        return &globalh[i];
  }
  return 0;
}

int
Router::hindex(const Element *e, const String &hname)
{
  if (e && e != e->router()->_root_element) {
    const Router *r = e->router();
    int eh = r->find_ehandler(e->eindex(), hname);
    if (eh >= 0)
      return r->_ehandler_to_handler[eh];
  } else {			// global handler
    for (int i = 0; i < nglobalh; i++)
      if (globalh[i]._name == hname)
        return FIRST_GLOBAL_HANDLER + i;
  }
  return -1;
}

void
Router::element_hindexes(const Element *e, Vector<int> &handlers)
{
  if (e && e != e->router()->_root_element) {
    const Router *r = e->router();
    for (int eh = r->_ehandler_first_by_element[e->eindex()];
         eh >= 0;
         eh = r->_ehandler_next[eh]) {
      int h = r->_ehandler_to_handler[eh];
      if (h >= 0)
        handlers.push_back(h);
    }
  } else {
    for (int i = 0; i < nglobal_handlers(); i++)
      handlers.push_back(FIRST_GLOBAL_HANDLER + i);
  }
}


// Public functions for storing handlers

void
Router::add_read_handler(const Element *e, const String &name,
			 ReadHandler read, void *thunk)
{
  Handler to_add = fetch_handler(e, name);
  to_add._read = read;
  to_add._read_thunk = thunk;
  store_handler(e, to_add);
}

void
Router::add_write_handler(const Element *e, const String &name,
			  WriteHandler write, void *thunk)
{
  Handler to_add = fetch_handler(e, name);
  to_add._write = write;
  to_add._write_thunk = thunk;
  store_handler(e, to_add);
}

int
Router::change_handler_flags(const Element *e, const String &name,
			     uint32_t clear_flags, uint32_t set_flags)
{
  Handler to_add = fetch_handler(e, name);
  if (to_add._use_count > 0) {	// only modify existing handlers
    to_add._flags = (to_add._flags & ~clear_flags) | set_flags;
    store_handler(e, to_add);
    return 0;
  } else
    return -1;
}


// global handlers

int
Router::nglobal_handlers()
{
  return nglobalh;
}


// ATTACHMENTS

void *
Router::attachment(const String &name) const
{
  for (int i = 0; i < _attachments.size(); i++)
    if (_attachment_names[i] == name)
      return _attachments[i];
  return 0;
}

void *&
Router::force_attachment(const String &name)
{
  for (int i = 0; i < _attachments.size(); i++)
    if (_attachment_names[i] == name)
      return _attachments[i];
  _attachment_names.push_back(name);
  _attachments.push_back(0);
  return _attachments.back();
}

void *
Router::set_attachment(const String &name, void *value)
{
  for (int i = 0; i < _attachments.size(); i++)
    if (_attachment_names[i] == name) {
      void *v = _attachments[i];
      _attachments[i] = value;
      return v;
    }
  _attachment_names.push_back(name);
  _attachments.push_back(value);
  return 0;
}

ErrorHandler *
Router::chatter_channel(const String &name) const
{
  if (!name || name == "default")
    return ErrorHandler::default_handler();
  else if (void *v = attachment("ChatterChannel." + name))
    return (ErrorHandler *)v;
  else
    return ErrorHandler::silent_handler();
}

int Router::new_notifier_signal(NotifierSignal &signal)
{
  if (!_notifier_signals)
    _notifier_signals = new atomic_uint32_t[NOTIFIER_SIGNALS_CAPACITY / 32];
  if (_n_notifier_signals >= NOTIFIER_SIGNALS_CAPACITY)
    return -1;
  else {
    signal = NotifierSignal(&_notifier_signals[_n_notifier_signals / 32], 1 << (_n_notifier_signals % 32));
    signal.set_active(true);
    _n_notifier_signals++;
    return 0;
  }
}

int ThreadSched::initial_thread_preference(Task *, bool)
{
  return 0;
}


// PRINTING

void Router::unparse_requirements(StringAccum &sa, const String &indent) const
{
  // requirements
  if (_requirements.size())
    sa << indent << "require(" << cp_unargvec(_requirements) << ");\n\n";
}

void Router::unparse_classes(StringAccum &, const String &) const
{
  // there are never any compound element classes here
}

void Router::unparse_declarations(StringAccum &sa, const String &indent) const
{  
  // element classes
  Vector<String> conf;
  for (int i = 0; i < nelements(); i++) {
    sa << indent << _element_names[i] << " :: " << _elements[i]->class_name();
    String conf = (initialized() ? _elements[i]->configuration() : _element_configurations[i]);
    if (conf.length())
      sa << "(" << conf << ")";
    sa << ";\n";
  }
  
  if (nelements() > 0)
    sa << "\n";
}

void Router::unparse_connections(StringAccum &sa, const String &indent) const
{  
  int nhookup = _hookup_from.size();
  Vector<int> next(nhookup, -1);
  Bitvector startchain(nhookup, true);
  for (int c = 0; c < nhookup; c++) {
    const Hookup &ht = _hookup_to[c];
    if (ht.port != 0) continue;
    int result = -1;
    for (int d = 0; d < nhookup; d++)
      if (d != c && _hookup_from[d] == ht) {
	result = d;
	if (_hookup_to[d].port == 0)
	  break;
      }
    if (result >= 0) {
      next[c] = result;
      startchain[result] = false;
    }
  }
  
  // print hookup
  Bitvector used(nhookup, false);
  bool done = false;
  while (!done) {
    // print chains
    for (int c = 0; c < nhookup; c++) {
      if (used[c] || !startchain[c]) continue;
      
      const Hookup &hf = _hookup_from[c];
      sa << indent << _element_names[hf.idx];
      if (hf.port)
	sa << " [" << hf.port << "]";
      
      int d = c;
      while (d >= 0 && !used[d]) {
	if (d == c) sa << " -> ";
	else sa << "\n" << indent << "    -> ";
	const Hookup &ht = _hookup_to[d];
	if (ht.port)
	  sa << "[" << ht.port << "] ";
	sa << _element_names[ht.idx];
	used[d] = true;
	d = next[d];
      }
      
      sa << ";\n";
    }

    // add new chains to include cycles
    done = true;
    for (int c = 0; c < nhookup && done; c++)
      if (!used[c])
	startchain[c] = true, done = false;
  }
}

void Router::unparse(StringAccum &sa, const String &indent) const
{
  unparse_requirements(sa, indent);
  unparse_classes(sa, indent);
  unparse_declarations(sa, indent);
  unparse_connections(sa, indent);
}

String Router::element_ports_string(int ei) const
{
  if (ei < 0 || ei >= nelements())
    return String();
  
  StringAccum sa;
  Element *e = _elements[ei];
  Vector<int> pers(e->ninputs() + e->noutputs(), 0);
  int *in_pers = pers.begin();
  int *out_pers = pers.begin() + e->ninputs();
  e->processing_vector(in_pers, out_pers, 0);

  sa << e->ninputs() << (e->ninputs() == 1 ? " input\n" : " inputs\n");
  for (int i = 0; i < e->ninputs(); i++) {
    // processing
    const char *persid = (e->input_is_pull(i) ? "pull" : "push");
    if (in_pers[i] == Element::VAGNOSTIC)
      sa << persid << "~\t";
    else
      sa << persid << "\t";
    
    // counts
#if CLICK_STATS >= 1
    if (e->input_is_pull(i) || CLICK_STATS >= 2)
      sa << e->input(i).npackets() << "\t";
    else
#endif
      sa << "-\t";
    
    // connections
    Hookup h(ei, i);
    const char *sep = "";
    for (int c = 0; c < _hookup_from.size(); c++)
      if (_hookup_to[c] == h) {
	sa << sep << _element_names[_hookup_from[c].idx]
	   << " [" << _hookup_from[c].port << "]";
	sep = ", ";
      }
    sa << "\n";
  }

  sa << e->noutputs() << (e->noutputs() == 1 ? " output\n" : " outputs\n");
  for (int i = 0; i < e->noutputs(); i++) {
    // processing
    const char *persid = (e->output_is_push(i) ? "push" : "pull");
    if (out_pers[i] == Element::VAGNOSTIC)
      sa << persid << "~\t";
    else
      sa << persid << "\t";
    
    // counts
#if CLICK_STATS >= 1
    if (e->output_is_push(i) || CLICK_STATS >= 2)
      sa << e->output(i).npackets() << "\t";
    else
#endif
      sa << "-\t";
    
    // hookup
    Hookup h(ei, i);
    const char *sep = "";
    for (int c = 0; c < _hookup_from.size(); c++)
      if (_hookup_from[c] == h) {
	sa << sep << "[" << _hookup_to[c].port << "] "
	   << _element_names[_hookup_to[c].idx];
	sep = ", ";
      }
    sa << "\n";
  }
  
  return sa.take_string();
}


// STATIC INITIALIZATION, DEFAULT GLOBAL HANDLERS

enum { GH_VERSION, GH_CONFIG, GH_FLATCONFIG, GH_LIST, GH_REQUIREMENTS };

String Router::router_read_handler(Element *e, void *thunk)
{
  Router *r = (e ? e->router() : 0);
  switch (reinterpret_cast<intptr_t>(thunk)) {

  case GH_VERSION:
    return String(CLICK_VERSION "\n");
    
  case GH_CONFIG:
    if (r)
      return r->configuration_string();
    break;

  case GH_FLATCONFIG:
    if (r) {
      StringAccum sa;
      r->unparse(sa);
      return sa.take_string();
    }
    break;

  case GH_LIST:
    if (r) {
      StringAccum sa;
      sa << r->nelements() << "\n";
      for (int i = 0; i < r->nelements(); i++)
        sa << r->_element_names[i] << "\n";
      return sa.take_string();
    }
    break;

  case GH_REQUIREMENTS:
    if (r) {
      StringAccum sa;
      for (int i = 0; i < r->_requirements.size(); i++)
        sa << r->_requirements[i] << "\n";
      return sa.take_string();
    }
    break;

  default:
    return "<error>\n";
    
  }
  return String();
}

static int stop_global_handler(const String &s,
                               Element *e,
                               void *,
                               ErrorHandler *errh)
{
  if (e) {
    int n = 1;
    (void) cp_integer(cp_uncomment(s), &n);
    e->router()->adjust_runcount(-n);
  } else
    errh->message("no router to stop");
  return 0;
}

void Router::static_initialize()
{
  if (!nglobalh) {
    add_read_handler(0, "version", router_read_handler, (void *)GH_VERSION);
    add_read_handler(0, "config", router_read_handler, (void *)GH_CONFIG);
    add_read_handler(0, "flatconfig", router_read_handler, (void *)GH_FLATCONFIG);
    add_read_handler(0, "list", router_read_handler, (void *)GH_LIST);
    add_read_handler(0, "requirements", router_read_handler, (void *)GH_REQUIREMENTS);
    add_write_handler(0, "stop", stop_global_handler, 0);
  }
}

void
Router::static_cleanup()
{
  delete[] globalh;
  globalh = 0;
  nglobalh = globalh_cap = 0;
}



REMOVABLE_INLINE bool
operator==(const Router::Hookup &a, const Router::Hookup &b)
{
  return a.idx == b.idx && a.port == b.port;
}

REMOVABLE_INLINE bool
operator!=(const Router::Hookup &a, const Router::Hookup &b)
{
  return a.idx != b.idx || a.port != b.port;
}

REMOVABLE_INLINE Element *
Router::find(const String &name, ErrorHandler *errh) const
{
  return find(name, "", errh);
}

REMOVABLE_INLINE const Router::Handler *
Router::handler(const Element *e, int hi)
{
  return handler(e ? e->router() : 0, hi);
}

REMOVABLE_INLINE const Router::Handler *
Router::handler(int hi) const
{
  return handler(this, hi);
}

REMOVABLE_INLINE
Router::Handler::Handler()
  : _read(0), _read_thunk(0), _write(0), _write_thunk(0),
     _flags(0), _use_count(0), _next_by_name(-1)
{
}

REMOVABLE_INLINE Router::Handler::Handler(const String &name)
  : _name(name),
    _read(0),
    _read_thunk(0),
    _write(0),
    _write_thunk(0),
    _flags(0),
    _use_count(0),
    _next_by_name(-1)
{
}

REMOVABLE_INLINE String Router::Handler::call_read(Element *e) const
{
  return _read(e, _read_thunk);
}

REMOVABLE_INLINE int Router::Handler::call_write(const String &s,
                                                 Element *e,
                                                 ErrorHandler *errh) const
{
  return _write(s, e, _write_thunk, errh);
}

REMOVABLE_INLINE bool Router::Handler::compatible(const Handler &h) const
{
  return (_read == h._read && _read_thunk == h._read_thunk
          && _write == h._write && _write_thunk == h._write_thunk
          && _flags == h._flags);
}

REMOVABLE_INLINE int Router::initial_thread_preference(Task *t,
                                                       bool scheduled) const
{
  if (!_thread_sched)
    return ThreadSched::THREAD_PREFERENCE_UNKNOWN;
  else
    return _thread_sched->initial_thread_preference(t, scheduled);
}
#endif
