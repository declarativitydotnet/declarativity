// -*- c-basic-offset: 2; related-file-name: "elementSpec.h" -*-
/*
 * @(#)$Id$
 *
 * Copyright (c) 1999-2000 Massachusetts Institute of Technology
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
 */

#include "elementSpec.h"

ElementSpec::ElementSpec(ElementRef e)
  : _element(e),
    _inputs(),
    _outputs()
{
  initializePorts();
}




void ElementSpec::initializePorts()
{
  // Start an iterator through the personality and flow specs
  const char * personalityPointer = _element->processing();
  const char * flowPointer = _element->flow_code();

  // Create the ports from the appropriate specs
  //////////////////////////////////////////////

  // Should I be advancing the pointers?
  bool pStop;
  bool fStop;

  // What is the default personality 
  Element::Processing pCurrent = Element::VAGNOSTIC;

  // Fill in the input port vector
  int ninputs = _element->ninputs();
  for (int i = 0;
       i < ninputs;
       i++) {
    // Better not be at the end of the specs
    assert((pStop != 0) && (fStop != 0));

    // Is this the end of the input descriptors?
    pStop = (*personalityPointer == '/');
    fStop = (*flowPointer == '/');

    // Should I change the current personality?
    if (!pStop) {
      pCurrent = Element::processingCode(personalityPointer);
      personalityPointer++;
    }

    // Create the port
    PortRef port = New refcounted< Port >(pCurrent);
    _inputs.push_back(port);
  }

  // I have all input ports set.  Proceed with the outputs

  // Skip the slashes.  There'd better be some
  assert((personalityPointer != 0) &&
         (flowPointer != 0) &&
         (*personalityPointer == '/') &&
         (*flowPointer == '/'));
  personalityPointer++;
  flowPointer++;

  // And start with defaults again
  pCurrent = Element::VAGNOSTIC;

  // Fill in the output port vector
  int noutputs = _element->noutputs();
  for (int i = 0;
       i < noutputs;
       i++) {
    // Is this the end of the input descriptors?
    pStop = (personalityPointer == 0);
    fStop = (flowPointer == 0);

    // Should I change the current personality?
    if (!pStop) {
      pCurrent = Element::processingCode(personalityPointer);
      personalityPointer++;
    }

    // Create the port
    PortRef port = New refcounted< Port >(pCurrent);
    _outputs.push_back(port);
  }
}
