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
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
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

  enum UnificationResult {
    PROGRESS,
    UNCHANGED,
    CONFLICT
  };

  
  ElementSpec(ElementPtr element);

  /** My real element */
  const ElementPtr element();

  /** An internal structure for tracking unification groups */
  struct UniGroup {
    std::vector< int > inputs;
    std::vector< int > outputs;
  };
  typedef boost::shared_ptr< UniGroup > UniGroupPtr;
  
  /** My unification groups */
  std::vector< UniGroupPtr > _uniGroups;

  /** A nested class encapsulating port specs. */
  class Port { 
  public:

    Port(Element::Processing personality);
    
    /** My personality */
    Element::Processing personality() const;
    
    /** Set my personality */
    void personality(Element::Processing);

    /** My uni group */
    UniGroupPtr uniGroup() const;

    /** Set my uni group, if I don't have one already */
    void uniGroup(UniGroupPtr);

    /** Unify this port with the given personality. Return a unification
        result.  A return value of CONFLICT means that no changes were
        made to this port. */
    UnificationResult unify(Element::Processing);

    /** Turn to string */
    string toString() const;
    
    /** My counterpart element */
    ElementPtr counterpart() const;

    /** Set my counterpart element.  Assumes the counterpart is not
        set. Return 1 if the counterpart was already set (without
        changing the counterpart), 0 otherwise. */
    int counterpart(ElementPtr);


  private:
    /** What's my personality? */
    Element::Processing _processing;

    /** What's my unification group, if any? */
    UniGroupPtr _uniGroup;

    /** What's my counterpart element? I should have no more, no less
        than one. */
    ElementPtr _counterpart;
  };

  typedef boost::shared_ptr< Port > PortPtr;

  /** My input */
  PortPtr input(int pno);

  /** My output */
  PortPtr output(int pno);
  
  /** A place holder exception for this class */
  struct ElementSpecError {};

  /** Turn spec to string */
  string toString() const;

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

  /** Apply unification to a single input port.  If the input port is
      agnostic, stop with no changes (i.e., don't back propagate from
      other ports to this one). */
  UnificationResult unifyInput(int portNumber);

  /** Apply unification to a single output port.  If the output port is
      agnostic, stop with no changes (i.e., don't back propagate from
      other ports to this one). */
  UnificationResult unifyOutput(int portNumber);

 private:

  /** My target element */
  ElementPtr _element;

  /** My input ports. No need for reference count since these do not move
      around at all but die with the object. */
  std::vector< PortPtr > _inputs;

  /** My output ports */
  std::vector< PortPtr > _outputs;

  ElementSpec(const Element &);
  ElementSpec &operator=(const Element &);
  
  /** Create my ports */
  void initializePorts();

  /** An auxilliary structure used for connecting flows. It is a vector
      of unification groups, one per flow code character.  */
  static std::vector< UniGroupPtr > _scratchUniGroups;
};


/** A handy dandy reference to element specs */
typedef boost::shared_ptr< ElementSpec > ElementSpecPtr;

#endif /* __ELEMENT_SPEC_H_ */
