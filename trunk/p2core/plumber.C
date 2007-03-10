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
#include <set>
#include "table2.h"
#include "reporting.h"

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

ElementSpecPtr Plumber::Dataflow::find(string name)
{
  for (std::vector<ElementSpecPtr>::iterator i = elements_.begin();
       i != elements_.end(); i++) {
    if ((*i)->element()->name() == name) {
      return *i;
    }
  }
  return ElementSpecPtr();
}


ElementSpecPtr Plumber::Dataflow::addElement(ElementPtr e) {
  ElementSpecPtr s(new ElementSpec(e));
  elements_.push_back(s);	// Add element to my dataflow
  return s;
}

void Plumber::Dataflow::hookUp(ElementSpecPtr src, int src_port,
                               ElementSpecPtr dst, int dst_port ) {
  assert(src != 0 && dst != 0);
  ElementSpec::HookupPtr p(new ElementSpec::Hookup(src, src_port, dst, dst_port));
  hookups_.push_back(p);
}


Table2Ptr
Plumber::Dataflow::table(string name,
                         Table2::Key& key,
                         uint32_t max_size,
                         string lifetime)
{
  std::map<string, Table2Ptr>::iterator iter = tables_.find(name);
  if (iter != tables_.end()) {
    // Already there. Just return it
    return iter->second;
  } else {
    Table2Ptr tp(new Table2(name, key, max_size, lifetime));
    tables_[name] = tp;
    return tp;
  }
}


Table2Ptr
Plumber::Dataflow::getTable(string name)
{
  std::map<string, Table2Ptr>::iterator iter = tables_.find(name);
  if (iter != tables_.end()) {
    return iter->second;
  }
  else {
    return Table2Ptr();
  }
}


int Plumber::Dataflow::check_hookup_elements()
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
    // TELL_INFO << "CHECKING HOOKUP: " << i << std::endl;
    ElementSpec::HookupPtr hookup = (hookups_)[i];
    // TELL_INFO << "FROM ELEMENT: " << hookup->fromElement->toString() << std::endl;
    // TELL_INFO << "TO ELEMENT: " << hookup->toElement->toString() << std::endl;

    std::set< ElementPtr >::iterator found =
      elementSet.find(hookup->fromElement->element());
    if ((found == elementSet.end()) ||
        (*found != hookup->fromElement->element())) {
      // This hookup comes from a non-existing element 
      TELL_ERROR << "Non-existent from element " <<
        hookup->fromElement->toString() << "\n";
      errors++;
    }
    found = elementSet.find(hookup->toElement->element());
    if ((found == elementSet.end()) ||
        (*found != hookup->toElement->element())) {
      // This hookup goes to a non-existing element 
      TELL_ERROR << "Non-existent to element " <<
        hookup->toElement->toString() << "\n";
      errors++;
    }
    if (hookup->fromPortNumber < 0) {
      // Negative port is bad
      TELL_ERROR << "Bad hookup from port " << hookup->fromPortNumber << "\n";
      errors++;
    }
    if (hookup->toPortNumber < 0) {
      // Negative port is bad
      TELL_ERROR << "Bad hookup to port " << hookup->toPortNumber << "\n";
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

int Plumber::Dataflow::check_push_and_pull()
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
      unsigned fromPort = hookup->fromPortNumber;
      unsigned toPort = hookup->toPortNumber;
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
          TELL_ERROR << "Hookup from PUSH["
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
            TELL_ERROR << "PUSH unification failed for element "
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
          TELL_ERROR << "Hookup from PULL["
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
            TELL_ERROR << "PULL unification failed for element "
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
            TELL_ERROR << "PUSH unification failed for element "
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
            TELL_ERROR << "PULL unification failed for element "
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
        TELL_ERROR << "Invalid personality for from element "
                  << fromElement->toString()
                  << " and port "
                  << fromPort
                  << "\n";
        errors++;
      }
    }	// END FOR
    
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


int Plumber::Dataflow::check_hookup_range()
{
  // Check each hookup to ensure its port numbers are within range
  int errors = 0;
  for (uint i = 0;
       i < hookups_.size();
       i++) {
    ElementSpec::HookupPtr hookup = (hookups_)[i];
    
    if (hookup->fromPortNumber >= hookup->fromElement->
        element()->noutputs()) {
      TELL_ERROR << "Cannot connect from port " <<
        hookup->fromPortNumber << " in element " <<
        hookup->fromElement->toString() << "\n";
      errors++;
    }
    if (hookup->toPortNumber >= hookup->toElement->
        element()->ninputs()) {
      TELL_ERROR << "Cannot connect to port " <<
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

int Plumber::Dataflow::eval_hookups()
{
  /** Set the counterpart port, check for duplicates along the way */
  int duplicates = 0;
  for (std::vector<ElementSpec::HookupPtr>::iterator iter = hookups_.begin();
       iter != hookups_.end(); iter++) {
    ElementSpec::HookupPtr hookup = *iter;
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;

    int dup =
      fromElement->output(fromPort)->counterpart(toElement);
    if (dup > 0) {
      TELL_ERROR << "Output port " << fromPort << " of element "
                << fromElement->toString()
                << " reused\n";
    }
    duplicates += dup;
    dup =
      toElement->input(toPort)->counterpart(fromElement);
    if (dup > 0) {
      TELL_ERROR << "Input port " << toPort << " of element "
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

int Plumber::Dataflow::check_hookup_completeness() {
  // Check unuseds
  ostringstream oss;
  int total = 0;
  for (std::vector<ElementSpecPtr>::iterator iter = elements_.begin();
       iter != elements_.end(); iter++) {
    ElementSpecPtr element = *iter;
    int unuseds = 0;
    for (unsigned in = 0;
         in < element->element()->ninputs();
         in++) {
      if (element->input(in)->check() && 
          element->input(in)->counterpart() == 0) {
        unuseds++;
        oss << "Input port " << in << " of element "
            << element->toString()
            << " unused\n";
      }
    }
    for (unsigned out = 0; out < element->element()->noutputs(); out++) {
      if (element->output(out)->check() && 
          element->output(out)->counterpart() == 0) {
        unuseds++;
        oss << "Output port " << out << " of element "
            << element->toString()
            << " unused\n";
      }
    }

    if (unuseds) {
      TELL_ERROR << oss.str();
      total += unuseds;
    }
  }
    
  if (total > 0) {
    return -1;
  } 
  return 0;
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
int Plumber::Dataflow::validate()
{
  // Are the hookups pointing to existing elements and ports?
  if (check_hookup_elements() < 0) {
    TELL_ERROR << "** Check_Hookup_Elements failed\n";
    return -1;
  }

  // Are the port numbers plausible?
  if (check_hookup_range() < 0) {
    TELL_ERROR << "** Port numbers implausible\n";
    return -1;
  }

  // Check push/pull semantics
  if (check_push_and_pull() < 0) {
    TELL_ERROR << "** Bad push/pull semantics\n";
    return -1;
  }

  // Evaluate hookups indicated in my new dataflow
  if (eval_hookups() < 0) {
    TELL_ERROR << "** Hookup evaluation failure\n";
    return -1;
  }

  // Check hookup completeness.  
  // All ports, in all dataflows, have something attached to them
  if (check_hookup_completeness() < 0) {
    TELL_ERROR << "** Hookup incompleteness\n";
    return -1;
  }

  return 0;
}

int Plumber::Dataflow::finalize() {
  set_connections();

  int failures = 0;
  for (std::vector<ElementSpecPtr>::iterator iter = elements_.begin();
       iter != elements_.end(); iter++) {
    if ((*iter)->element()->state()  == Element::INACTIVE && 
        (*iter)->element()->initialize() < 0) {
      TELL_ERROR << "** Initialize element " 
                << (*iter)->element()->name() 
                << " failure.\n";
      failures++;
    }
    else {
      (*iter)->element()->state(Element::ACTIVE);
    }
  }
  return (failures == 0) ? 0 : -1;
}

/****************************************************
 * Plumber::DataflowEdit methods
 */

Plumber::DataflowEdit::DataflowEdit(DataflowPtr d) : Dataflow(d->name()) 
{
  tables_           = d->tables_;		// Copy over the tables
  garbage_elements_ = d->garbage_elements_;	// Copy over the garbage elements

  /** I really need to call the 'addElement' method here so that a new
    * ElementSpectPtr is created around the old Element structure */
  for (std::vector<ElementSpecPtr>::iterator iter = d->elements_.begin();
       iter != d->elements_.end(); iter++) {
    this->Dataflow::addElement((*iter)->element());
  }

  /** I also want new Hookups (without the old counterparts) so that the
    * validation step does not think I'm reusing ports */
  for (std::vector<ElementSpec::HookupPtr>::iterator iter = d->hookups_.begin();
       iter != d->hookups_.end(); iter++) {
    ElementSpec::HookupPtr hookup = *iter;
    int fromIndex = -1;
    int toIndex   = -1;
    for (unsigned i = 0; i < d->elements_.size(); i++) {
      ElementSpecPtr e = d->elements_[i];
      if (hookup->fromElement == e) {
        fromIndex = int(i);
        break;
      }
    }
    for (unsigned i = 0; i < d->elements_.size(); i++) {
      ElementSpecPtr e = d->elements_[i];
      if (hookup->toElement == e) {
        toIndex = int(i);
        break;
      }
    }
    assert(fromIndex >= 0 && toIndex >= 0);
    assert(fromIndex < int(elements_.size()) && toIndex < int(elements_.size()));
    this->Dataflow::hookUp(elements_[fromIndex], hookup->fromPortNumber, 
                           elements_[toIndex], hookup->toPortNumber);
  }
}

ElementSpecPtr Plumber::DataflowEdit::addElement(ElementPtr e) {
  ElementSpecPtr s(new ElementSpec(e));
  new_elements_.push_back(s);	// Add element to my dataflow
  return s;
}

void Plumber::DataflowEdit::hookUp(ElementSpecPtr src, int src_port,
                                   ElementSpecPtr dst, int dst_port ) {
  assert(src != 0 && dst != 0);
  ElementSpec::HookupPtr p(new ElementSpec::Hookup(src, src_port, dst, dst_port));
  new_hookups_.push_back(p);
  remove_old_hookups(p);
}

void Plumber::DataflowEdit::disconnect_output(ElementSpecPtr e, int port)
{
  port = e->remove_output(port);
  if (port < 0)
    return;

  for(std::vector<ElementSpec::HookupPtr>::iterator iter = hookups_.begin();
      iter != hookups_.end(); iter++) {
    if ((*iter)->fromElement    == e && 
        (*iter)->fromPortNumber == unsigned(port)) {
      hookups_.erase(iter);
      return;
    }
  }
}

void Plumber::DataflowEdit::disconnect_output(ElementSpecPtr e, ValuePtr portKey)
{
  int port = e->element()->output(portKey);
  if (port >= 0)
    disconnect_output(e, port);
}

void Plumber::DataflowEdit::disconnect_input(ElementSpecPtr e, int port)
{
  port = e->remove_input(port);
  if (port < 0)
    return;

  for(std::vector<ElementSpec::HookupPtr>::iterator iter = hookups_.begin();
      iter != hookups_.end(); iter++) {
    if ((*iter)->toElement    == e && 
        (*iter)->toPortNumber == unsigned(port)) {
      hookups_.erase(iter);
      return;
    }
  }
}

void Plumber::DataflowEdit::disconnect_input(ElementSpecPtr e, ValuePtr portKey)
{
  int port = e->element()->input(portKey);
  if (port >= 0)
    disconnect_input(e, port);
}

void Plumber::DataflowEdit::set_connections()
{
  // actually assign ports
  for (std::vector<ElementSpec::HookupPtr>::iterator iter = new_hookups_.begin();
       iter != new_hookups_.end(); iter++) {
    ElementSpec::HookupPtr hookup = *iter;
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

int Plumber::DataflowEdit::validate() {
  // Toss all the new hookups into the mix
  for(std::vector<ElementSpec::HookupPtr>::iterator iter = new_hookups_.begin();
      iter != new_hookups_.end(); iter++) {
    hookups_.push_back(*iter);
  }

  /* If an *old* element does not have any hookups referencing an input, 
   * or if it doesn't have inputs then any hookups referencing an output, 
   * then it was part of some edit. Given that we've put the new hookups in 
   * the mix, if this Element still has 0 input hookups then it will be removed
   * from the dataflow. Any output hookups that reference the element deemed
   * to be removed will also be taken out so that the element removal process
   * can occur transitively. */ 
  bool fully_connected;
  do {
    fully_connected = true;	// Assume everyone is fully connected
    for(std::vector<ElementSpecPtr>::iterator eiter = elements_.begin();
        eiter != elements_.end(); ) {
      bool element_referenced_in_hookup = false;
      if ((*eiter)->element()->ninputs() > 0) {
        // Check the inputs for a hookup reference
        for(std::vector<ElementSpec::HookupPtr>::iterator hiter = hookups_.begin();
            hiter != hookups_.end(); hiter++) {
          if ((*hiter)->toElement == *eiter) {
            element_referenced_in_hookup = true;
            break;
          } 
        }
      }
      if ((*eiter)->element()->ninputs() == 0 && (*eiter)->element()->noutputs() > 0) {
        /* It doesn't have inputs so check so ensure some output 
         * has a hookup reference. */
        for(std::vector<ElementSpec::HookupPtr>::iterator hiter = hookups_.begin();
            hiter != hookups_.end(); hiter++) {
          if ((*hiter)->fromElement == *eiter) {
            element_referenced_in_hookup = true;
            break;
          } 
        }
      }
      if (!element_referenced_in_hookup) {
        fully_connected = false;
        /* Remove any hookups that this element has that may not have been part of
         * the actual edit. This could be an element within a subgraph of an edit. */
        for(std::vector<ElementSpec::HookupPtr>::iterator hiter = hookups_.begin();
            hiter != hookups_.end(); ) {
          if ((*hiter)->toElement == *eiter || (*hiter)->fromElement == *eiter) {
            hiter = hookups_.erase(hiter);
          }  else hiter++;
        }
        /* Erase this element from the dataflow (but make sure it is not deleted
         * in case it has any outstanding callbacks) */
        (*eiter)->element()->state(Element::INACTIVE);
        garbage_elements_.push_back(*eiter);	// Don't deleted from memory
        eiter = elements_.erase(eiter);
      } else eiter++;
    }
  } while (!fully_connected);
  

  /** At this point I add the *new* elements into the mix */
  for(std::vector<ElementSpecPtr>::iterator iter = new_elements_.begin();
      iter != new_elements_.end(); iter++) {
    elements_.push_back(*iter);
  }

  return this->Dataflow::validate();
}

void Plumber::DataflowEdit::remove_old_hookups(ElementSpec::HookupPtr hookup)
{
  for(std::vector<ElementSpec::HookupPtr>::iterator iter = hookups_.begin();
      iter != hookups_.end(); iter++) {
    if ((*iter)->fromElement    == hookup->fromElement && 
        (*iter)->fromPortNumber == hookup->fromPortNumber) {
      hookups_.erase(iter);
      break;
    }
  }
  for(std::vector<ElementSpec::HookupPtr>::iterator iter = hookups_.begin();
      iter != hookups_.end(); iter++) {
    if ((*iter)->toElement    == hookup->toElement && 
        (*iter)->toPortNumber == hookup->toPortNumber) {
      hookups_.erase(iter);
      break;
    }
  }
}


ElementSpecPtr Plumber::DataflowEdit::find(string name)
{
  for (std::vector<ElementSpecPtr>::iterator i = elements_.begin();
       i != elements_.end(); i++) {
    if ((*i)->element()->name() == name) {
      return *i;
    }
  }
  return ElementSpecPtr();
}

/********************************************************
 * Plumber is responsible for maintaining a set of dataflows
 * each defined by their configuration. 
 */

Plumber::Plumber()
  : _dataflows(new std::map<string, DataflowPtr>()),
    _logger(0)
{
}

/**
 * install
 */
int Plumber::install(DataflowPtr d)
{
  if (d->validate() < 0 || d->finalize() < 0) {
    TELL_ERROR << "** Dataflow installation failure\n";
    return -1;
  }

  std::map<string, DataflowPtr>::iterator iter = _dataflows->find(d->name());
  if (iter != _dataflows->end()) {
    _dataflows->erase(iter);
  }
  _dataflows->insert(std::make_pair(d->name(), d));
  return 0;
}


void Plumber::toDot(string f) { 
  TELL_INFO << "Outputting DOT for dataflow into file "
            << f
            << "\n";
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

REMOVABLE_INLINE LoggerI * Plumber::logger(LoggerI * newLogger)
{
  LoggerI * l = _logger;
  _logger = newLogger;
  return l;
}
