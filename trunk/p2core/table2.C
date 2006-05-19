/*
 * @(#)$Id$
 *
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Next generation table implementation.
 *
 */

#include "table2.h"


////////////////////////////////////////////////////////////
// Sorters
////////////////////////////////////////////////////////////

/** My comparator for vectors of unsigned integers. */
bool
Table2::unsignedVectorLess::operator()(const Table2::Key first,
                                       const Table2::Key second) const
{
  if (first.size() < second.size()) {
    return true; // first < second
  } else if (first.size() > second.size()) {
    return false; // second < first
  } else {
    Table2::Key::const_iterator firstIt = first.begin();
    Table2::Key::const_iterator secondIt = second.begin();
    while (firstIt != first.end()) {
      if ((*firstIt) >= (*secondIt)) {
        // We're done, second >= first
        return false;
      }
    }
    // We're done, first < second
    return true;
  }
}


/** My comparator for value ptr vectors. */
bool
Table2::valuePtrVectorLess::
operator()(const Table2::ValuePtrVector first,
           const Table2::ValuePtrVector second) const
{
  if (first.size() < second.size()) {
    return true; // first < second
  } else if (first.size() > second.size()) {
    return false; // second < first
  } else {
    Table2::ValuePtrVector::const_iterator firstIt = first.begin();
    Table2::ValuePtrVector::const_iterator secondIt = second.begin();
    while (firstIt != first.end()) {
      if ((*firstIt)->compareTo(*secondIt) >= 0) {
        // We're done, second >= first
        return false;
      }
    }
    // We're done, first < second
    return true;
  }
}


