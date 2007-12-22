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

#include "dataflowEdit.h"

/****************************************************
 * DataflowEdit methods
 */

DataflowEdit::DataflowEdit(DataflowPtr d)
  : Dataflow(d->name()) 
{
  garbage_elements_ = d->garbage_elements_;	// Copy over the garbage
  // elements

  /** I really need to call the 'addElement' method here so that a new
    * ElementSpectPtr is created around the old Element structure */
  for (std::vector<ElementSpecPtr>::iterator iter = d->elements_.begin();
       iter != d->elements_.end(); iter++) {
    ElementPtr e = (*iter)->element();
    Dataflow* d = NULL;
    if ((d = dynamic_cast<Dataflow*>(e.get())) != NULL) {
      /* We must clear the inputs and outputs to internal dataflow element */
      for (unsigned i = 0; i < d->ninputs(); i++) {
        unsigned port = i;
        d->input(&port)->input(port)->reset();
      }
      for (unsigned o = 0; o < d->noutputs(); o++) {
        unsigned port = o;
        d->output(&port)->output(port)->reset();
      }
    }
    this->Dataflow::addElement(e);
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


ElementSpecPtr
DataflowEdit::addElement(ElementPtr e) {
  for (std::vector<ElementSpecPtr>::iterator i = new_elements_.begin();
       i != new_elements_.end(); i++) {
    if ((*i)->element().get() == e.get()) return *i;
  }

  ElementSpecPtr s(new ElementSpec(e));
  new_elements_.push_back(s);	// Add element to my dataflow
  return s;
}


void
DataflowEdit::hookUp(ElementSpecPtr src, unsigned src_port,
                                   ElementSpecPtr dst, unsigned dst_port ) {
  assert(src != 0 && dst != 0);
  ElementSpec::HookupPtr p(new ElementSpec::Hookup(src, src_port, dst, dst_port));
  new_hookups_.push_back(p);
  remove_old_hookups(p);
}

void DataflowEdit::disconnect_output(ElementSpecPtr e, int port)
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

void DataflowEdit::disconnect_output(ElementSpecPtr e, ValuePtr portKey)
{
  int port = e->element()->output(portKey);
  if (port >= 0)
    disconnect_output(e, port);
}

void DataflowEdit::disconnect_input(ElementSpecPtr e, int port)
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

void DataflowEdit::disconnect_input(ElementSpecPtr e, ValuePtr portKey)
{
  int port = e->element()->input(portKey);
  if (port >= 0)
    disconnect_input(e, port);
}

void DataflowEdit::set_connections()
{
  // actually assign ports
  for (std::vector<ElementSpec::HookupPtr>::iterator iter = new_hookups_.begin();
       iter != new_hookups_.end(); iter++) {
    ElementSpec::HookupPtr hookup = *iter;
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

int DataflowEdit::validate() {
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

void DataflowEdit::remove_old_hookups(ElementSpec::HookupPtr hookup)
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

