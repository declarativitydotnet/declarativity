// -*- c-basic-offset: 2; -*-
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
 *  Utility functions to produce a DOT description of a plumber
 *  configuration.
 *
 *  Petros Maniatis.
 */

#include <plumber.h>
#include <element.h>
#include <elementSpec.h>
#include <iostream>

void
toDot(std::ostream * ostr,
      Plumber::ConfigurationPtr configuration)
{
  *ostr << "digraph G {\n"
        << "rankdir=LR;\n"
        << "node [shape=record];\n";

  // Delcare all elements.
  for (uint e = 0;
       e < configuration->elements.size();
       e++) {
    ElementPtr element = configuration->elements[e]->element();
    *ostr << element->ID()      // unique element ID
          << " [ label=\"{";

    // Now figure out how many input ports
    if (element->ninputs() > 0) {
      *ostr << "{<i0> 0";
      for (int p = 1;
           p < element->ninputs();
           p++) {
        *ostr << "| <i" << p << "> " << p << " ";
      }
      *ostr << "}|";
    }
      
    // Show the name
    *ostr << element->class_name() // the official type
          << "\\n"
          << element->name();   // the official name

    // And figure out the output ports
    if (element->noutputs() > 0) {
      *ostr << "|{<o0> 0";
      for (int p = 1;
           p < element->noutputs();
           p++) {
        *ostr << "| <o" << p << "> " << p << " ";
      }
      *ostr << "}";
    }

    // Close the element label
    *ostr << "}\" ];\n";
  }

  for (uint i = 0;
       i < configuration->hookups.size();
       i++) {
    Plumber::HookupPtr hookup = configuration->hookups[i];
    ElementSpecPtr fromElement = hookup->fromElement;
    ElementSpecPtr toElement = hookup->toElement;
    int fromPort = hookup->fromPortNumber;
    int toPort = hookup->toPortNumber;
    *ostr << fromElement->element()->ID() << ":" << "o" << fromPort
          << " -> "
          << toElement->element()->ID() << ":" << "i" << toPort
          << ";\n";
  }
  *ostr << "}\n";
  ostr->flush();
}
