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
 * DESCRIPTION: A directory for all defined aggregate functions.
 *
 */

#ifndef __AGGFACTORY_H__
#define __AGGFACTORY_H__

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "map"
#include "commonTable.h"

class AggFactory
{
public:
  /** A type for aggregate function factories.  Just a boost
      function. */
  typedef boost::function< CommonTable::AggFunc* () > AggFuncFactory;

  
  /** Ensure we're initialized.  All exterior-facing functions dealing
      with statics must invoke this first */
  static void
  ensureInit();


  /** Returns a new aggregate function object on the heap. Must be
      deleted in the end.  It never returns a null. If the given name is
      not found, an AggregateNotFound exception is raised. */
  static CommonTable::AggFunc*
  mk(std::string aggName);


  /** Returns an aggregate function factory.  If the given name is not
      found, an AggregateNotFound exception is raised. */
  static AggFuncFactory
  factory(std::string aggName);


  /** Returns a string containing all aggregates known to me */
  static std::string
  aggList();


  /** An exception thrown when a factory fails to locate a named
      aggregate function */
  struct AggregateNotFound {
  public:
    AggregateNotFound(std::string name);
    
    
    std::string aggName;
  };


  /** Registers an aggregate function factory given its name (e.g., MIN,
      MAX, etc.) */
  static bool
  add(std::string aggName,
      AggFuncFactory factory);




  
  
private:
  /** A type for a directory of aggregate function factories. */
  typedef std::map< std::string, // name
                    AggFuncFactory, // factory
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



#define DECLARE_PRIVATE_INITS                   \
  class Initializer {                           \
  public:                                       \
    Initializer();                              \
  };                                            \
                                                \
  static Initializer*                           \
  theInitializer();                             \
                                                \


#define DECLARE_PUBLIC_INITS(_classname) \
  static _classname*                     \
  mk();                                  \
                                         \
  std::string                            \
  name();                                \
                                         \
  static void                            \
  ensureInit();                          \
                                         \
  

#define DEFINE_INITS(_classname,_name)         \
  _classname::Initializer::Initializer()       \
  {                                            \
    AggFactory::add(_name, &_classname::mk);   \
  }                                            \
                                               \
  _classname::Initializer*                     \
  _classname::theInitializer()                 \
  {                                            \
    static Initializer* _initializer =         \
      new Initializer();                       \
    return _initializer;                       \
  }                                            \
                                               \
  void                                         \
  _classname::ensureInit()                     \
  {                                            \
    Initializer* init;                         \
    init = theInitializer();                   \
  }                                            \
                                               \
  _classname*                                  \
  _classname::mk()                             \
  {                                            \
    return new _classname();                   \
  }                                            \
                                               \
  std::string                                  \
  _classname::name()                           \
  {                                            \
    return _name;                              \
  }                                            \
                                               \
  


#endif // AGGMIN_H
