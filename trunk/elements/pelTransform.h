// -*- c-basic-offset: 2; related-file-name: "pelTransform.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that runs a PEL program on inputs
 */

#ifndef __PELTRANSFORM_H__
#define __PELTRANSFORM_H__

#include "element.h"
#include "pel_program.h"
#include "pel_vm.h"

class PelTransform : public Element { 
public:
  PelTransform(str, str);

  ~PelTransform();
  
  /** Overridden to perform the tranformation. */
  TuplePtr simple_action(TupleRef p);

  const char *class_name() const		{ return "PelTransform";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** My program.  */
  Pel_Program * _program;

  /** The virtual machine within which to execute the transform.  Any
      need to share this? */
  Pel_VM _vm;
};


#endif /* __PELTRANSFORM_H_ */
