// -*- c-basic-offset: 2; related-file-name: "noNullField.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that only passes through tuples whose named
 * field is a non-empty tuple.
 */

#ifndef __NONULLFIELD_H__
#define __NONULLFIELD_H__

#include "element.h"

class NoNullField : public Element { 
public:
  NoNullField(str, unsigned);
  ~NoNullField();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TupleRef p);

  const char *class_name() const		{ return "NoNullField";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }

private:
  /** The field number to check */
  unsigned _fieldNo;
};


#endif /* __NONULLFIELD_H_ */
