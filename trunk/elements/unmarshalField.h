// -*- c-basic-offset: 2; related-file-name: "unmarshalField.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that unmarshals a single field in-place within a
 * tuple.  The field in question must be of type OPAQUE.
 */

#ifndef __UNMARSHALFIELD_H__
#define __UNMARSHALFIELD_H__

#include "element.h"

class UnmarshalField : public Element { 
public:
  UnmarshalField(str, unsigned);

  ~UnmarshalField();
  
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "UnmarshalField";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** The field number I'm unmarshalling */
  unsigned _fieldNo;
};


#endif /* __UNMARSHALFIELD_H_ */
