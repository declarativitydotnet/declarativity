// -*- c-basic-offset: 2; related-file-name: "filter.C" -*-
/*
 * @(#)$Id$
 * 
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * 
 * DESCRIPTION: Element that drops tuples according to a simple boolean
 * predicate.  The predicate field must be of type INT32. 0 drops while
 * any other INT32 value keeps the field.  A non-INT32 field or the
 * absence of the field drops the tuple.
 */

#ifndef __FILTER_H__
#define __FILTER_H__

#include "element.h"

class Filter : public Element { 
public:

  Filter(str, unsigned);

  /** Overridden to perform the filtering. */
  TuplePtr simple_action(TuplePtr p);

  const char *class_name() const		{ return "Filter";}
  const char *processing() const		{ return "a/a"; }
  const char *flow_code() const			{ return "x/x"; }


private:
  /** My filter field number.*/
  unsigned _filterNo;
};


#endif /* __FILTER_H_ */
