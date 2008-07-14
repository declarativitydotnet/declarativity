// -*- c-basic-offset: 2; related-file-name: "elementSpec.C" -*-
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
 * DESCRIPTION: This is a representation of an element used during
 * configuration.
 */

#ifndef __ELEMENT_SPEC_H__
#define __ELEMENT_SPEC_H__

#include <set>
#include "inlines.h"
#include "element.h"

class ElementSpec;
/** A handy dandy reference to element specs */
typedef boost::shared_ptr< ElementSpec > ElementSpecPtr;

class ElementSpec { 
 public:
  enum UnificationResult {
    PROGRESS,
    UNCHANGED,
    CONFLICT
  };

  /** An auxilliary structure to help pass around element connection
      specifications */
  struct Hookup {
    string toString() {
      ostringstream oss;
      oss << ((fromElement->element()) ? fromElement->element()->class_name() : "None")
          << "[" << fromPortNumber << "]" << " -> "
          << "[" << toPortNumber << "]"
          << ((toElement->element()) ? toElement->element()->class_name() : "None")
          << std::endl;
      return oss.str();
    };

    /** The dataflow name that owns this hookup (there can be only 1). */
    string _dataflow;

    /** The element from which this hookup originates */
    ElementSpecPtr fromElement;

    /** The port number at the fromElement */
    int fromPortNumber;

    /**  The element to which this hookup goes */
    ElementSpecPtr toElement;

    /** The port number at the toElement */
    int toPortNumber;

    Hookup(string d, 
           ElementSpecPtr fe, int fp,
           ElementSpecPtr te, int tp)
      : _dataflow(d), fromElement(fe), fromPortNumber(fp),
        toElement(te), toPortNumber(tp) {};
  };
  typedef boost::shared_ptr< Hookup > HookupPtr;

  
  ElementSpec(ElementPtr element);

  /** My real element */
  const ElementPtr element();

  /** Add to the list of dataflows that reference this element */
  void dataflowRef(string name) { _dataflowRefs->insert(name); }
  
  /** Return my dataflow ref vector */
  const std::set<string>* dataflowRefs() { return _dataflowRefs.get(); }

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

    /** Reset the port */
    void reset() { _counterpart.reset(); }

    /** Turn to string */
    string toString() const;
    
    /** My counterpart element */
    ElementSpecPtr counterpart() const;

    /** Set my counterpart element.  Assumes the counterpart is not
        set. Return 1 if the counterpart was already set (without
        changing the counterpart), 0 otherwise. */
    int counterpart(ElementSpecPtr, HookupPtr);

    /** Return the hookup that made this port */
    HookupPtr hookup() const { return _hookup; }

  private:
    /** What's my personality? */
    Element::Processing _processing;

    /** What's my unification group, if any? */
    UniGroupPtr _uniGroup;

    /** What's my counterpart element? I should have no more, no less
        than one. */
    ElementSpecPtr _counterpart;

    /** The hookup that made this connection */
    HookupPtr _hookup;
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
  boost::shared_ptr<std::vector< PortPtr > > _inputs;

  /** My output ports */
  boost::shared_ptr<std::vector< PortPtr > > _outputs;

  /** This vector holds the dataflow names that contain this object */
  boost::shared_ptr<std::set< string > > _dataflowRefs;

  ElementSpec(const Element &);
  ElementSpec &operator=(const Element &);
  
  /** Create my ports */
  void initializePorts();

  /** An auxilliary structure used for connecting flows. It is a vector
      of unification groups, one per flow code character.  */
  static std::vector< UniGroupPtr > _scratchUniGroups;
};

#endif /* __ELEMENT_SPEC_H_ */
