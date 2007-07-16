// -*- c-basic-offset: 2; related-file-name: "pelTransform.C" -*-
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
 * DESCRIPTION: Element that runs a PEL program on inputs
 */

#ifndef __PELTRANSFORM_H__
#define __PELTRANSFORM_H__

#include "element.h"
#include "elementRegistry.h"
#include "pel_program.h"
#include "pel_vm.h"

class PelTransform : public Element { 
public:
  PelTransform(string, string);
  PelTransform(TuplePtr args);

  ~PelTransform() {};
  
  /** Overridden to perform the tranformation. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "PelTransform";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


  /** What's my program code? */
  string
  pelCode();

  DECLARE_PUBLIC_ELEMENT_INITS

private:
  /** My program.  */
  boost::shared_ptr< Pel_Program > _program;

  /** My code */
  string _pelCode;

  /** The virtual machine within which to execute the transform.  Any
      need to share this? */
  Pel_VM _vm;

  DECLARE_PRIVATE_ELEMENT_INITS
};


#endif /* __PELTRANSFORM_H_ */
