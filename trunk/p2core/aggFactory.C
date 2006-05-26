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
 */

#include "aggFactory.h"
#include <boost/function.hpp>

// Aggregate functions
#include "aggMin.h"
#include "aggMax.h"
#include "aggCount.h"

Table2::AggFunc*
AggFactory::mk(std::string aggName)
{
  // Look up the constructor method
  FactorySet::iterator i = _factories.find(aggName);

  // Do we have it?
  if (i == _factories.end()) {
    // Nope. Return null
    return NULL;
  } else {
    // Execute it and return the function object
    return ((*i).second)();
  }
}


bool
AggFactory::add(std::string aggName,
                AggFuncFactory factory)
{
  // Just insert it and return true if it is new
  bool succeeded = _factories.insert(std::make_pair(aggName, factory)).second;

  return succeeded;
}


AggFactory::Initializer::Initializer()
{
  // Register all known factories
  
  add("MIN", &AggMin::mk);
  add("MAX", &AggMax::mk);
  add("COUNT", &AggCount::mk);
}
