// -*- c-basic-offset: 2; related-file-name: "dot.h" -*-
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
 */

#include <element.h>
#include <elementSpec.h>
#include <iostream>
#include "val_int64.h"
#include "reporting.h"

void
toDot(std::ostream * ostr,
      const std::set<ElementSpecPtr>& elements, 
      const std::set<ElementSpec::HookupPtr>& hookups, bool header = true)
{
  if(header){
    *ostr << "digraph G {\n"
          << "rankdir=LR;\n"
          << "node [shape=record];\n";
  }

  // Delcare all elements.
  for (std::set<ElementSpecPtr>::const_iterator i = elements.begin();
       i != elements.end(); i++) {
    //TELL_INFO<<"toDotting "<<(*i)->element()->name() <<" \n";
    (*i)->element()->toDot(ostr);
  }

  for (std::set<ElementSpec::HookupPtr>::const_iterator i = hookups.begin();
       i != hookups.end(); i++) {
    const ElementSpec::HookupPtr hookup = *i;
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;
    *ostr << fromElement->element()->ID() << ":" << "o" << fromPort
          << " -> "
          << toElement->element()->ID() << ":" << "i" << toPort
          << ";\n";
  }

  if(header){
    *ostr << "}\n";
  }
  ostr->flush();
}
