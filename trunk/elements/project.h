// -*- c-basic-offset: 2; related-file-name: "project.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that projects a tuple to only some of its
 * fields.  The field mask to project onto is represented via a bit
 * string.
 */

#ifndef __PROJECT_H__
#define __PROJECT_H__

#include "element.h"
#include <bitset>

class Project : public Element { 
public:

  /** Bit masks are defined to have a maximum number of bits */
  static const uint MAX_NUMBER_OF_FIELDS = 256;

  /** The type of projection mask is a bit set */
  typedef std::bitset<MAX_NUMBER_OF_FIELDS> Spec;

  Project(Spec spec);

  ~Project();
  
  /** Overridden to perform the projecting. */
  TuplePtr simple_action(TupleRef p);

  const char *class_name() const		{ return "Project";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** The spec of my projection */
  Spec _spec;
};


#endif /* __PROJECT_H_ */
