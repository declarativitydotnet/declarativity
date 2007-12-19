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

#include "stageRegistry.h"
#include "oniStageProcessor.h"
#include <boost/function.hpp>
#include "stage.h"


StageRegistry::StageNotFound::StageNotFound(std::string name)
  : stageName(name)
{
}


Stage::Processor*
StageRegistry::mk(std::string stageProcessorName,
                  Stage* theStageElement)
{
  ensureInit();

  // Look up the constructor method
  FactorySet::iterator i = _factories->find(stageProcessorName);

  // Do we have it?
  if (i == _factories->end()) {
    // Nope.  Try to dynamically link to it. (throws StageNotFound exception)
    return OniStageFactory(stageProcessorName, theStageElement); 
  } else {
    // Execute it to create the processor
    Stage::Processor* processor = ((*i).second)(theStageElement);

    return processor;
  }
}


bool
StageRegistry::add(std::string stageName,
                   StageFactory factory)
{
  ensureInit();

  // Just insert it and return true if it is new
  bool succeeded =
    _factories->insert(std::make_pair(stageName,
                                      factory)).second;

  return succeeded;
}


std::string
StageRegistry::stageList()
{
  ensureInit();

  std::string stageList;
  for (FactorySet::iterator i = _factories->begin();
       i != _factories->end();
       i++) {
    std::string currentName = (*i).first;
    stageList += currentName + ";";
  }
  
  return stageList;
}


StageRegistry::FactorySet*
StageRegistry::_factories;


/** Return the initializer ensuring it runs first */
StageRegistry::Initializer*
StageRegistry::theInitializer()
{
  static Initializer* _initializer =
    new Initializer();
  return _initializer;
}


StageRegistry::Initializer::Initializer()
{
  _factories = new FactorySet();
  OniStageFactoryInit();
}


/** Implements the construct at first use pattern */
void
StageRegistry::ensureInit()
{
  Initializer* init;
  init = theInitializer();
}


