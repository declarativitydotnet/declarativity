// -*- c-basic-offset: 2; related-file-name: "project.h" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 */

#include "project.h"

Project::Project(Project::Spec spec)
  : Element(1, 1),
    _spec(spec)
{
}

Project::~Project()
{
}

TuplePtr Project::simple_action(TupleRef p)
{
  // Check if the tuple has more fields than the maximum projection
  uint noFields = p->size();
  if (noFields > Project::MAX_NUMBER_OF_FIELDS) {
    // OK, only attempt to project the first MAX_NUMBER_OF_FIELDS
    noFields = Project::MAX_NUMBER_OF_FIELDS;
  }

  // Create the new tuple
  TupleRef newTuple = Tuple::mk();

  // For all tuple fields
  for (uint field = 0;
       field < noFields;
       field++) {
    // Is the spec bit on?
    if (_spec[field]) {
      // Include a copy of the tuple field
      newTuple->append((*p)[field]);
    } else {
      // Skip this tuple field
    }
  }

  // Does this tuple contain any fields?
  if (newTuple->size() > 0) {
    
    // Freeze it and return it
    newTuple->freeze();
    
    return newTuple;
  } else {
    // Nope, this is empty.
    return 0;
  }
}
