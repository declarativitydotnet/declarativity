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
#include <ddemux.h>
#include <demux.h>
#include "val_int32.h"
#include "val_uint32.h"

void
toDot(std::ostream * ostr,
      const std::set<ElementSpecPtr>& elements, 
      const std::set<ElementSpec::HookupPtr>& hookups)
{
  *ostr << "digraph G {\n"
        << "rankdir=LR;\n"
        << "node [shape=record];\n";

  // Delcare all elements.
  for (std::set<ElementSpecPtr>::const_iterator i = elements.begin();
       i != elements.end(); i++) {
    const ElementPtr element = (*i)->element();
    *ostr << element->ID()      // unique element ID
          << " [ label=\"{";

    // Now figure out how many input ports
    if (element->ninputs() > 0) {
      *ostr << "{<i0> 0";
      for (unsigned p = 1;
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

    // And figure out the output ports.  For demux, put the names of the
    // output in there as well
    if (element->noutputs() > 0) {
      if (string(element->class_name()).compare("DDemux") == 0) {
        DDemux* demuxPtr = (DDemux*) element.get();
        
        *ostr << "|{";

        DDemux::PortMap::iterator miter =
          demuxPtr->_port_map.begin();
        while (miter != demuxPtr->_port_map.end()) {
          *ostr << "<o"
                << miter->second
                << "> "
                << miter->first->toString();

          miter++;
          *ostr << "|";
        }
        *ostr << "<o0> default";
        *ostr << "}";
      } else if (string(element->class_name()).compare("Demux") == 0) {
        Demux* demuxPtr = (Demux*) element.get();
        
        *ostr << "|{";

        std::vector< ValuePtr >::iterator miter =
          demuxPtr->_demuxKeys.begin();
        uint counter = 0;
        while (miter != demuxPtr->_demuxKeys.end()) {
          *ostr << "<o"
                << counter
                << "> "
                << (*miter)->toString();

          miter++;
          counter++;
          *ostr << "|";
        }
        *ostr << "<o" << counter << "> default";
        *ostr << "}";
      } else {
        *ostr << "|{<o0> 0";
        for (unsigned p = 1;
             p < element->noutputs();
             p++) {
          *ostr << "| <o" << p << "> " << p << " ";
        }
        *ostr << "}";
      }
    }

    // Close the element label
    *ostr << "}\" ];\n";
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
  *ostr << "}\n";
  ostr->flush();
}
