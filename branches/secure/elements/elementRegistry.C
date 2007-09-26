/*
 * Copyright (c) 2005 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 */

#include "elementRegistry.h"
#include <boost/function.hpp>
#include "element.h"
#include<iostream>

ElementRegistry::ElementNotFound::ElementNotFound(std::string name)
  : elementName(name)
{
}


ElementPtr
ElementRegistry::mk(std::string elementName,
                    TuplePtr args)
{
  ensureInit();

  // Look up the constructor method
  FactoryMap::iterator i = _factories->find(elementName);

  // Do we have it?
  if (i == _factories->end()) {
    // Nope. Throw exception
    std::cout<<"ELEMENT NOT FOUND"<<elementName;
    throw ElementNotFound(elementName);
  } else {
    // Execute it to create the element
    ElementPtr element = ((*i).second)(args);

    return element;
  }
}


bool
ElementRegistry::add(std::string elementName,
                     ElementFactory factory)
{
  ensureInit();

  // Just insert it and return true if it is new
  bool succeeded =
    _factories->insert(std::make_pair(elementName,
                                      factory)).second;
  
  return succeeded;
}


std::string
ElementRegistry::elementList()
{
  ensureInit();

  std::string elementList;
  for (FactoryMap::iterator i = _factories->begin();
       i != _factories->end();
       i++) {
    std::string currentName = (*i).first;
    elementList += currentName + ";";
  }
  
  return elementList;
}


ElementRegistry::FactoryMap*
ElementRegistry::_factories;


/** Return the initializer ensuring it runs first */
ElementRegistry::Initializer*
ElementRegistry::theInitializer()
{
  static Initializer* _initializer =
    new Initializer();
  return _initializer;
}


ElementRegistry::Initializer::Initializer()
{
  _factories = new FactoryMap();
}


/** Implements the construct at first use pattern */
void
ElementRegistry::ensureInit()
{
  Initializer* init;
  init = theInitializer();
}


