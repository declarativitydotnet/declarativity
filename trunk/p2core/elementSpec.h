// -*- c-basic-offset: 2; related-file-name: "elementSpec.C" -*-
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
 * 
 * DESCRIPTION: This is a representation of an element used during
 * configuration.
 */

#ifndef __ELEMENT_SPEC_H__
#define __ELEMENT_SPEC_H__

#include "inlines.h"
#include "element.h"

class ElementSpec { 
 public:
  class PortSpec;
  
  ElementSpec(ElementRef element);

  // INPUTS AND OUTPUTS
  int ninputs() const				{ return _ninputs; }
  int noutputs() const				{ return _noutputs; }

  /** My real element */
  const ElementRef element()			{ return _element; }

  /** A nested class encapsulating port specs. */
  class Port { 
  public:
    
    Port(Element::Processing personality) : _processing(personality) {};
    
    /** My personality */
    Element::Processing personality() const { return _processing; }
    
   private:
    /** What's my personality? */
    Element::Processing _processing;
  };

  typedef ref< Port > PortRef;

  /** A place holder exception for this class */
  struct ElementSpecError {};

 private:

  /** My target element */
  ElementRef _element;

  /** My input ports. No need for refcountedness since these do not move
      around at all but die with the object. */
  vec< PortRef > _inputs;

  /** My output ports */
  vec< PortRef > _outputs;

  /** How many inputs do I have? */
  int _ninputs;

  /** How many outputs do I have? */
  int _noutputs;

  ElementSpec(const Element &);
  ElementSpec &operator=(const Element &);
  
  /** Create my ports */
  void initializePorts();
};


/** A handy dandy reference to element specs */
typedef ref< ElementSpec > ElementSpecRef;

#endif /* __ELEMENT_SPEC_H_ */
