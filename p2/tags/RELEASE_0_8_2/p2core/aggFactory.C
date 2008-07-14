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

AggFactory::AggregateNotFound::AggregateNotFound(std::string name)
  : aggName(name)
{
}


AggFactory::AggregateNotFound::~AggregateNotFound()
  throw()
{
}


const char*
AggFactory::AggregateNotFound::what()
{
  return aggName.c_str();
}



CommonTable::AggFunc*
AggFactory::mk(std::string aggName)
{
  ensureInit();

  // Look up the constructor method
  FactorySet::iterator i = _factories->find(aggName);

  // Do we have it?
  if (i == _factories->end()) {
    // Nope. Throw exception
    TELL_ERROR << "Aggregate factory known functions: " << std::endl;
    for (i = _factories->begin(); i != _factories->end(); i++)
      TELL_ERROR << "\t" << i->first << std::endl;
    throw AggregateNotFound(aggName);
  } else {
    // Execute it and return the function object
    return ((*i).second)();
  }
}


AggFactory::AggFuncFactory
AggFactory::factory(std::string aggName)
{
  ensureInit();

  // Look up the constructor method
  FactorySet::iterator i = _factories->find(aggName);

  // Do we have it?
  if (i == _factories->end()) {
    // Nope. Throw exception
    TELL_ERROR << "Aggregate factory known functions: " << std::endl;
    for (i = _factories->begin(); i != _factories->end(); i++)
      TELL_ERROR << "\t" << i->first << std::endl;
    exit(0);
    throw AggregateNotFound(aggName);
  } else {
    // Return the factory
    return ((*i).second);
  }
}


bool
AggFactory::add(std::string aggName,
                AggFuncFactory factory)
{
  ensureInit();

  // Just insert it and return true if it is new
  bool succeeded = _factories->insert(std::make_pair(aggName, factory)).second;

  return succeeded;
}


std::string
AggFactory::aggList()
{
  ensureInit();

  std::string aggList;
  for (FactorySet::iterator i = _factories->begin();
       i != _factories->end();
       i++) {
    std::string currentName = (*i).first;
    aggList += currentName + ";";
  }
  
  return aggList;
}


AggFactory::FactorySet*
AggFactory::_factories;


/** Return the initializer ensuring it runs first */
AggFactory::Initializer*
AggFactory::theInitializer()
{
  static Initializer* _initializer =
    new Initializer();
  return _initializer;
}


AggFactory::Initializer::Initializer()
{
  _factories = new FactorySet();
}


/** Implements the construct at first use pattern */
void
AggFactory::ensureInit()
{
  Initializer* init;
  init = theInitializer();
}


