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

  /** My real element */
  const ElementRef element();

  /** A nested class encapsulating port specs. */
  class Port { 
  public:
    
    Port(Element::Processing personality);
    
    /** My personality */
    Element::Processing personality() const;
    
    /** Set my personality */
    void personality(Element::Processing);

    /** Turn to string */
    str toString() const;
    
  private:
    /** What's my personality? */
    Element::Processing _processing;
  };

  typedef ref< Port > PortRef;

  /** My input */
  PortRef input(int pno);

  /** My output */
  PortRef output(int pno);
  
  /** A place holder exception for this class */
  struct ElementSpecError {};

  /** Turn spec to string */
  str toString() const;

  /** Turn processing code to processing string */
  REMOVABLE_INLINE static char * processingCodeString(Element::Processing p) {
    switch (p) {
    case Element::PUSH:
      return "h";
    case Element::PULL:
      return "l";
    case Element::AGNOSTIC:
      return "a";
    default:
      return "I";               /* for invalid */
    }
  }

 private:

  /** My target element */
  ElementRef _element;

  /** My input ports. No need for refcountedness since these do not move
      around at all but die with the object. */
  vec< PortRef > _inputs;

  /** My output ports */
  vec< PortRef > _outputs;

  ElementSpec(const Element &);
  ElementSpec &operator=(const Element &);
  
  /** Create my ports */
  void initializePorts();
};


/** A handy dandy reference to element specs */
typedef ref< ElementSpec > ElementSpecRef;

#endif /* __ELEMENT_SPEC_H_ */
