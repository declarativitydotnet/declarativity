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

#ifndef __STAGEREGISTRY_H__
#define __STAGEREGISTRY_H__

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "map"
#include "stage.h"

class StageRegistry
{
public:
  /** A type for stage factories.  Just a boost function. */
  typedef boost::function< Stage::Processor* (Stage*) > StageFactory;

  
  /** Returns a new processor on the heap.  It never returns a null. If
      the given processor name is not found, StageNotFound exception is
      raised. */
  static Stage::Processor*
  mk(std::string stageProcessorName,
     Stage* theStageElement);


  /** Returns a string containing all stage processors known to me */
  static std::string
  stageList();


  /** An exception thrown when a factory fails to locate a named
      stage. */
  struct StageNotFound {
  public:
    StageNotFound(std::string name);
    
    std::string stageName;
  };


  /** Registers a stage factory given its user-visible name (e.g., SORT,
      etc..) */
  static bool
  add(std::string stageName,
      StageFactory factory);




  
  
private:
  /** Ensure we're initialized.  All exterior-facing functions dealing
      with statics must invoke this first */
  static void
  ensureInit();


  /** A type for a directory of stage factories. */
  typedef std::map< std::string, // name
                    StageFactory, // factory
                    std::less< std::string > > FactorySet;
  

  /** The actual directory */
  static FactorySet* _factories;


  /** A static initializer object to initialize static class objects */
  class Initializer {
  public:
    Initializer();
  };

  
  /** Return the initializer ensuring it runs first */
  static Initializer*
  theInitializer();
};



#define DECLARE_PRIVATE_STAGE_INITS             \
  class Initializer {                           \
  public:                                       \
    Initializer();                              \
  };                                            \
                                                \
  static Initializer*                           \
  theInitializer();                             \
                                                \


#define DECLARE_PUBLIC_STAGE_INITS(_classname)  \
  static _classname*                            \
  mk(Stage*);                                   \
                                                \
  std::string                                   \
  class_name();                                 \
                                                \
  static void                                   \
  ensureInit();                                 \
                                                \
  

#define DEFINE_STAGE_INITS(_classname,_name)         \
  _classname::Initializer::Initializer()             \
  {                                                  \
    StageRegistry::add(_name, &_classname::mk);      \
  }                                                  \
                                                     \
  _classname::Initializer*                           \
  _classname::theInitializer()                       \
  {                                                  \
    static Initializer* _initializer =               \
      new Initializer();                             \
    return _initializer;                             \
  }                                                  \
                                                     \
  void                                               \
  _classname::ensureInit()                           \
  {                                                  \
    Initializer* init;                               \
    init = theInitializer();                         \
  }                                                  \
                                                     \
  _classname*                                        \
  _classname::mk(Stage* theStageElement)             \
  {                                                  \
    return new _classname(theStageElement);          \
  }                                                  \
                                                     \
  std::string                                        \
  _classname::class_name()                           \
  {                                                  \
    return _name;                                    \
  }                                                  \
                                                     \
  


#endif // __STAGEREGISTRY_H__
