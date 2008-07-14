// -*- c-basic-offset: 2; related-file-name: "plumber.h" -*-
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
 * DESCRIPTION: The plumber class, for creating dataflow graphs
 * 
 */


#include <plumber.h>
#include <iostream>
#include <set>

/**************************************************************
 * Plumber::Dataflow is responsible for a dataflow that
 * is to be installed into the plumber. 
 */

std::string Plumber::Dataflow::toString() const {
    ostringstream oss;
    for (std::vector<ElementSpecPtr>::const_iterator i = elements_.begin();
         i != elements_.end(); i++) {
      oss << (*i)->toString() << std::endl;
    }
    return oss.str();
}

ElementSpecPtr Plumber::Dataflow::addElement(ElementPtr e) {
  ElementSpecPtr s(new ElementSpec(e));
  s->dataflowRef(name());	// Add my dataflow name
  elements_.push_back(s);	// Add element to my dataflow
  return s;
}

ElementSpecPtr Plumber::Dataflow::addElement(string d, string n) {
  ElementSpecPtr s = plumber_->resolve(d, n);
  if (s == 0) { 
    return s; 	// Not able to resolve element.
  }
  s->dataflowRef(name());	// Add my dataflow name
  elements_.push_back(s);	// Add element to my dataflow
  return s;
}

void Plumber::Dataflow::hookUp(ElementSpecPtr src, int src_port,
                               ElementSpecPtr dst, int dst_port ) {
  ElementSpec::HookupPtr p(new ElementSpec::Hookup(name(), src, src_port, dst, dst_port));
  hookups_.push_back(p);
}

void Plumber::Dataflow::remove(ElementSpec::HookupPtr hpt) { 
  std::vector<ElementSpec::HookupPtr>::iterator iter = 
    std::find(hookups_.begin(), hookups_.end(), hpt);
  if (iter != hookups_.end())
    hookups_.erase(iter); 
}

void Plumber::Dataflow::remove(ElementSpecPtr esp) { 
  std::vector<ElementSpecPtr>::iterator iter =
    std::find(elements_.begin(), elements_.end(), esp); 
  if (iter != elements_.end())
    elements_.erase(iter); 
}

ElementSpecPtr Plumber::Dataflow::find(string name)
{
  for (std::vector<ElementSpecPtr>::iterator i = elements_.begin();
       i != elements_.end(); i++) {
    assert((*i)->element());
    if ((*i)->element()->name() == name) {
      return *i;
    }
  }
  return ElementSpecPtr();
}

int Plumber::Dataflow::check_hookup_elements() const
{
  // Put all (real not spec) elements in a set to be searchable
  std::set< ElementPtr > elementSet;
  for (uint i = 0;
       i < elements_.size();
       i++) {
    elementSet.insert((elements_)[i]->element());
  }
  
  // Check each hookup to ensure it connects valid element references
  int errors = 0;
  for (uint i = 0;
       i < hookups_.size();
       i++) {
    ElementSpec::HookupPtr hookup = (hookups_)[i];
    std::set< ElementPtr >::iterator found =
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

int Plumber::Dataflow::check_push_and_pull() const
{
  int errors = 0;
  // For every hookup...
  ElementSpec::UnificationResult result;
  while (1) {
    bool progress = false;

    for (uint i = 0;
         i < hookups_.size();
         i++) {
      ElementSpec::HookupPtr hookup = (hookups_)[i];
      
      ElementSpecPtr fromElement = hookup->fromElement;
      ElementSpecPtr toElement = hookup->toElement;
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
          std::cerr << "Hookup from PUSH["
                    << fromElement->toString()
                    << "] port "
                    << fromPort
                    << " to PULL["
                    << toElement->toString()
                    << "] port "
                    << toPort
                    << " found\n";
          errors++;
          break;
          
        case Element::AGNOSTIC:
          // If to port is agnostic, make it push and unify it
          toElement->input(toPort)->personality(Element::PUSH);
          ElementSpec::UnificationResult result =
            toElement->unifyInput(toPort);
          if (result == ElementSpec::CONFLICT) {
            std::cerr << "PUSH unification failed for element "
                      << toElement->toString()
                      << "\n";
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
          std::cerr << "Hookup from PULL["
                    << fromElement->toString()
                    << "] port "
                    << fromPort
                    << " to PUSH["
                    << toElement->toString()
                    << "] port "
                    << toPort
                    << " found\n";
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
            std::cerr << "PULL unification failed for element "
                      << toElement->toString()
                      << "\n";
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
            std::cerr << "PUSH unification failed for element "
                      << fromElement->toString()
                      << "\n";
            errors++;
          };
          progress = true;
          break;
          
        case Element::PULL:
          // If to port is pull, make from pull
          fromElement->output(fromPort)->personality(Element::PULL);
          result = fromElement->unifyOutput(fromPort);
          if (result == ElementSpec::CONFLICT) {
            std::cerr << "PULL unification failed for element "
                      << fromElement->toString()
                      << "\n";
            errors++;
          };
          progress = true;
          break;
          
        case Element::AGNOSTIC:
          // If to port is agnostic, we're ok
          break;
        }
        break;

      default:
        std::cerr << "Invalid personality for from element "
                  << fromElement->toString()
                  << " and port "
                  << fromPort
                  << "\n";
        errors++;
      }
    }
    
    if (!progress) {
      break;
    }
  } 

  if (errors > 0) {
    return -1;
  } else {
    return 0;
  }
}


int Plumber::Dataflow::check_hookup_range() const
{
  // Check each hookup to ensure its port numbers are within range
  int errors = 0;
  for (uint i = 0;
       i < hookups_.size();
       i++) {
    ElementSpec::HookupPtr hookup = (hookups_)[i];
    
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

void Plumber::Dataflow::set_connections()
{
  // actually assign ports
  for (uint i = 0; i < hookups_.size(); i++) {
    ElementSpec::HookupPtr hookup = hookups_[i];
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;

    fromElement->element()->connect_output(fromPort,
                                           toElement->element().get(),
                                           toPort);
    toElement->element()->connect_input(toPort,
                                        fromElement->element().get(),
                                        fromPort);
  }
}


/**
 * validate
 *
 * This performs a simplified initialization of the element topology:
 *
 * - Check that connections refer to correct elements
 *
 * - Skip configuration ordering, since for now we start with
 * preconfigured elements.
 *
 */
int Plumber::Dataflow::validate() const 
{
  // Are the hookups pointing to existing elements and ports?
  if (check_hookup_elements() < 0) {
    std::cerr << "** Check_Hookup_Elements failed";
    return -1;
  }

  // Are the port numbers plausible?
  if (check_hookup_range() < 0) {
    std::cerr << "** Port numbers implausible";
    return -1;
  }

  // Check push/pull semantics
  if (check_push_and_pull() < 0) {
    std::cerr << "** Bad push/pull semantics";
    return -1;
  }

  return 0;
}

void Plumber::Dataflow::finalize() {
  set_connections();
  for (std::vector<ElementSpecPtr>::iterator iter = elements_.begin();
       iter != elements_.end(); iter++)
    (*iter)->element()->initialize();
}


/********************************************************
 * Plumber is responsible for maintaining a set of dataflows
 * each defined by their configuration. 
 */

Plumber::Plumber(LoggerI::Level loggingLevel)
  : loggingLevel(loggingLevel),
    _dataflows(new std::map<string, DataflowPtr>()),
    _logger(0)
{
}

/**
 * install
 */
int Plumber::install(DataflowPtr d)
{
  if (d->validate() < 0) {
    std::cerr << "** Dataflow installation failure\n";
    return -1;
  }

  // Evaluate hookups indicated in my new dataflow
  if (eval_hookups(d) < 0) {
    std::cerr << "** Hookup evaluation failure";
    return -1;
  }

  // Check hookup completeness.  
  // All ports, in all dataflows, have something attached to them
  if (check_hookup_completeness(d->name()) < 0) {
    std::cerr << "** Hookup incompleteness";
    return -1;
  }

  d->finalize();
  _dataflows->insert(std::make_pair(d->name(), d));
  return 0;
}

void Plumber::disconnect(ElementSpec::HookupPtr hookup)
{
  (hookup)->fromElement->output((hookup)->fromPortNumber)->reset();
  (hookup)->toElement->input((hookup)->toPortNumber)->reset(); 
  dataflow(hookup->_dataflow)->remove(hookup); 
}

int Plumber::eval_hookups(DataflowPtr d)
{
  /* If hookup refers to an element with an already connected
     port then disconnect the hold port. */
  for (uint i = 0;
       i < d->hookups_.size();
       i++) {
    ElementSpec::HookupPtr hookup = d->hookups_[i];
    
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;
    
    // Remove the hookups that once refered to other endpoints
    if (fromElement->output(fromPort)->counterpart()) {
      disconnect(fromElement->output(fromPort)->hookup());
    }
    if (toElement->input(toPort)->counterpart()) {
      disconnect(toElement->input(toPort)->hookup());
    }
  }

  /** Set the counterpart port, check for duplicates along the way */
  int duplicates = 0;
  for (uint i = 0;
       i < d->hookups_.size();
       i++) {
    ElementSpec::HookupPtr hookup = d->hookups_[i];
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;

    int dup =
      fromElement->output(fromPort)->counterpart(toElement, hookup);
    if (dup > 0) {
      std::cerr << "Output port " << fromPort << " of element "
                << fromElement->toString()
                << " reused\n";
    }
    duplicates += dup;
    dup =
      toElement->input(toPort)->counterpart(fromElement, hookup);
    if (dup > 0) {
      std::cerr << "Input port " << toPort << " of element "
                << toElement->toString()
                << " reused\n";
    }
    duplicates += dup;
  }
  if (duplicates > 0) {
    return -1;
  } else {
    return 0;
  }
}

int Plumber::check_hookup_completeness(string installedDataflowName) {
  std::vector<ElementSpecPtr> garbage;

  for (std::map<string, Plumber::DataflowPtr>::iterator iter = _dataflows->begin();
       iter != _dataflows->end(); iter++) {
    std::vector< ElementSpecPtr > elements = iter->second->elements_;
    std::ostringstream oss;
    // Check unuseds
    int total = 0;
    for (uint i = 0;
         i < elements.size();
         i++) {
      ElementSpecPtr element = (elements)[i];
      int unuseds = 0;
      for (int in = 0;
           in < element->element()->ninputs();
           in++) {
        if (element->input(in)->counterpart() == 0) {
          unuseds++;
          oss << "Input port " << in << " of element "
              << element->toString()
              << " unused\n";
        }
      }
      for (int out = 0;
           out < element->element()->noutputs();
           out++) {
        if (element->output(out)->counterpart() == 0) {
          unuseds++;
          oss << "Output port " << out << " of element "
              << element->toString()
              << " unused\n";
        }
      }
      if (installedDataflowName != iter->second->name() &&
          unuseds == element->element()->ninputs()+element->element()->noutputs()) {
        garbage.push_back(element);
      }
      else if (unuseds) {
        std::cerr << oss;
        total += unuseds;
      }
    }
    
    if (total > 0) {
      return -1;
    } else {
      for (std::vector<ElementSpecPtr>::iterator i = garbage.begin(); 
           i != garbage.end(); i++) {
        remove(*i);	// Garbage collect the element
      }
    }
  }
  return 0;
}

void Plumber::remove(ElementSpecPtr e) {
  const std::set<string>* refs = e->dataflowRefs();
  for (std::set<string>::const_iterator i = refs->begin();
       i != refs->end(); i++) {
    DataflowPtr dpt = dataflow(*i);
    assert(dpt);
    dpt->remove(e);
  } 
}

void Plumber::toDot(string f) { 
  std::set<ElementSpec::HookupPtr> hookups;
  std::set<ElementSpecPtr> elements;

  for (std::map<string, DataflowPtr>::iterator iter = _dataflows->begin();
       iter != _dataflows->end(); iter++) {
    std::vector<ElementSpec::HookupPtr>& h = iter->second->hookups_;
    std::vector<ElementSpecPtr>&         e = iter->second->elements_;
    for (unsigned i = 0; i < e.size(); i++) {
      elements.insert(e[i]);
    }
    for (unsigned i = 0; i < h.size(); i++) {
      hookups.insert(h[i]);
    }
  }
  std::ofstream ostr(f.c_str());
  ::toDot(&ostr, elements, hookups);
}

ElementSpecPtr Plumber::resolve(string d, string n) {
  DataflowPtr dpt = dataflow(d);
  if (dpt == 0) {
    std::cerr << "Dataflow " << d << " does not exist." << std::endl;
    return ElementSpecPtr();
  }
  ElementSpecPtr e = dpt->find(n);
  if (e == 0) {
    std::cerr << "Element in dataflow " << d << " with name "
              << n << " does not exist." << std::endl;
    return ElementSpecPtr();
  }
  return e;
}

REMOVABLE_INLINE LoggerI * Plumber::logger(LoggerI * newLogger)
{
  LoggerI * l = _logger;
  _logger = newLogger;
  return l;
}
