/*
 * @(#)$Id: plumber.C 1716 2007-12-19 01:52:11Z maniatis $
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

#include "dataflow.h"


/**************************************************************
 * Dataflow is responsible for a dataflow that
 * is to be installed into the plumber. 
 */
Dataflow::Dataflow(string name, 
                   unsigned ninputs,
                   unsigned noutputs, 
                   string p,
                   string fc) 
  : Element(name, ninputs, noutputs), 
    _processing(p.c_str()),
    _flow_code(fc.c_str()),
    _validated(false),
    _initialized(false) 
{
}

Dataflow::~Dataflow() { }

void
Dataflow::toDot(std::ostream* dotstr)
{
  Element::toDot(dotstr);
  return;
  /*std::set<ElementSpec::HookupPtr> hookups;
  std::set<ElementSpecPtr> elements;

  for(int i=0;i<elements_.size();i++)
	  elements.insert(elements_[i]);
  for(int i=0;i<hookups_.size();i++)
	  hookups.insert(hookups_[i]);
  std::ostringstream subg;
  subg<<"subgraph "<<ID()<<"{\n";
  ::toDot(&subg, elements,hookups,false);
  subg<<"\n};";
  *dotstr<<subg.str();*/
}


std::string Dataflow::toString() const {
    ostringstream oss;
    for (std::vector<ElementSpecPtr>::const_iterator i = elements_.begin();
         i != elements_.end(); i++) {
      oss << (*i)->toString() << std::endl;
    }
    return oss.str();
}

ElementSpecPtr
Dataflow::input(unsigned* port)
{
  assert (*port < inputs_.size());
  for (std::vector<ElementSpec::HookupPtr>::iterator i = inputs_.begin();
       i != inputs_.end(); i++)
    if ((*i)->fromPortNumber == *port) {
      Dataflow *d = NULL;
      /* Modify port mapping to refer to element's input port number. */
      *port = (*i)->toPortNumber; 
      if ((d = dynamic_cast<Dataflow*>((*i)->toElement->element().get())) != NULL) {
        return d->input(port);
      }
      return (*i)->toElement;
    }
  return ElementSpecPtr();
}

bool
Dataflow::checkInput(ElementSpecPtr element, unsigned port)
{
  for (std::vector<ElementSpec::HookupPtr>::iterator i = inputs_.begin();
       i != inputs_.end(); i++) {
    if ((*i)->toPortNumber == port && (*i)->toElement == element) {
      return true;
    }
  }
  return false;
}

ElementSpecPtr
Dataflow::output(unsigned* port)
{
  if (!(*port < outputs_.size())) {
    TELL_ERROR << "DATAFLOW PORT ERROR: " << name() << " invalid port " << *port << std::endl;
    assert(0);
  }

  for (std::vector<ElementSpec::HookupPtr>::iterator i = outputs_.begin();
       i != outputs_.end(); i++)
    if ((*i)->toPortNumber == *port) {
      Dataflow *d = NULL;
      /* Modify port mapping to refer to element's output port number. */
      *port = (*i)->fromPortNumber; 
      if ((d = dynamic_cast<Dataflow*>((*i)->fromElement->element().get())) != NULL) {
        return d->output(port);
      }
      return (*i)->fromElement;
    }
  return ElementSpecPtr();
}

bool
Dataflow::checkOutput(ElementSpecPtr element, unsigned port)
{
  for (std::vector<ElementSpec::HookupPtr>::iterator i = outputs_.begin();
       i != outputs_.end(); i++) {
    if ((*i)->fromPortNumber == port && (*i)->fromElement == element) {
      return true;
    }
  }
  return false;
}

/*
int Dataflow::connect_input(unsigned i, Element *f, unsigned port)
{
  if (i >= 0 && i < _inputs.size()) {
    return input(&i)->element()->connect_input(i, f, port);
    // _inputs[i].reset(new Port(this, f, port));
  } else
    return -1;
}

int Dataflow::connect_output(unsigned o, Element *f, unsigned port)
{
  if (o >= 0 && o < _outputs.size()) {
    return output(&o)->element()->connect_output(o, f, port);
    // _outputs[o].reset(new Port(this, f, port));
  } else
    return -1;
}
*/

ElementSpecPtr 
Dataflow::find(string name)
{
  for (std::vector<ElementSpecPtr>::iterator i = elements_.begin();
       i != elements_.end(); i++) {
    if ((*i)->element()->name() == name) {
      return *i;
    }
  }

  return ElementSpecPtr();
}


ElementSpecPtr Dataflow::addElement(ElementPtr e) {
  for (std::vector<ElementSpecPtr>::iterator i = elements_.begin();
       i != elements_.end(); i++) {
    if ((*i)->element().get() == e.get()) return *i;
  }

  ElementSpecPtr s(new ElementSpec(e));
  elements_.push_back(s);	// Add element to my dataflow

  return s;
}

void Dataflow::hookUp(ElementSpecPtr src, unsigned src_port,
                               ElementSpecPtr dst, unsigned dst_port ) {
  ElementSpec::HookupPtr p(new ElementSpec::Hookup(src, src_port, dst, dst_port));
  if (src && dst) {
    hookups_.push_back(p); // Internal hookup
  }
  else if (dst) {
    inputs_.push_back(p);  // Input hookup (only destination specified).
  }
  else if (src) {
    outputs_.push_back(p); // Output hookup (only src specified).
  }
  else {
    assert(0);
  }
}


int Dataflow::check_hookup_elements()
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
        hookup->fromElement->toString() << " in dataflow " << name() << "\n";
      errors++;
    }
    found = elementSet.find(hookup->toElement->element());
    if ((found == elementSet.end()) ||
        (*found != hookup->toElement->element())) {
      // This hookup goes to a non-existing element 
      TELL_ERROR << "Non-existent to element " <<
        hookup->toElement->toString() << " in dataflow " << name() << "\n";
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

int Dataflow::check_push_and_pull()
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


int Dataflow::check_hookup_range()
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

int Dataflow::eval_hookups()
{
  /** Set the counterpart port, check for duplicates along the way */
  int duplicates = 0;
  for (std::vector<ElementSpec::HookupPtr>::iterator iter = hookups_.begin();
       iter != hookups_.end(); iter++) {
    ElementSpec::HookupPtr hookup = *iter;
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    unsigned fromPort = hookup->fromPortNumber;
    unsigned toPort = hookup->toPortNumber;

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

int Dataflow::check_hookup_completeness() {
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
      if (checkInput(element, in)) {
        continue;
      }
      else if (element->input(in)->check() && 
          element->input(in)->counterpart() == 0) {
        unuseds++;
        oss << "Input port " << in << " of element "
            << element->toString()
            << " unused\n";
      }
    }
    for (unsigned out = 0; out < element->element()->noutputs(); out++) {
      if (checkOutput(element, out)) {
        continue;
      }
      else if (element->output(out)->check() && 
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

void Dataflow::set_connections()
{
  // actually assign ports
  for (uint i = 0; i < hookups_.size(); i++) {
    ElementSpec::HookupPtr hookup = hookups_[i];
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    unsigned fromPort = hookup->fromPortNumber;
    unsigned toPort = hookup->toPortNumber;

    Dataflow *d = NULL;
    if ((d = dynamic_cast<Dataflow*>(fromElement->element().get())) != NULL) {
      fromElement = d->output(&fromPort);
    }
    if ((d = dynamic_cast<Dataflow*>(toElement->element().get())) != NULL) {
      toElement = d->input(&toPort);
    }
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
int Dataflow::validate()
{
  if (!_validated) {
    /** Validate any sub-dataflow elements first */
    for (std::vector<ElementSpecPtr>::iterator iter = elements_.begin();
         iter != elements_.end(); iter++) {
      Dataflow* d = NULL;
      if ((d = dynamic_cast<Dataflow*>((*iter)->element().get())) != NULL &&
          d->validate() < 0)
        return -1;
    }
  
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
  }
  _validated = true;

  return 0;
}

int Dataflow::initialize() {
  int failures = 0;
  if (!_initialized) {
    set_connections();

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
	//ensure all elements are ACTIVE
        (*iter)->element()->state(Element::ACTIVE);
      }
    }
  }

  if (failures == 0) {
    _initialized = true;
    return 0;
  }
  return -1;
}

ElementPtr
Dataflow::injector()
{
  return _injector;
}


void
Dataflow::injector(ElementPtr injector)
{
  _injector = injector;
}



