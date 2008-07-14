// -*- c-basic-offset: 2; related-file-name: "marshalField.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that marshals a single field in-place within a
 * tuple.  The field in question must be of type TUPLE.
 */

#ifndef __MARSHALFIELD_H__
#define __MARSHALFIELD_H__

#include "element.h"

class MarshalField : public Element { 
public:
  MarshalField(str, unsigned);

  ~MarshalField();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TupleRef p);

  const char *class_name() const		{ return "MarshalField";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** The field number I'm marshalling */
  unsigned _fieldNo;
};


#endif /* __MARSHALFIELD_H_ */
