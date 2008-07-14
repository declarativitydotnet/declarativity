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

#ifndef __ELEMENTREGISTRY_H__
#define __ELEMENTREGISTRY_H__

#include "element.h"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include "boost/bind.hpp"
#include <map>


class ElementRegistry
{
public:
  /** A type for element factories. Just a boost function. */
  typedef boost::function< ElementPtr (TuplePtr) > ElementFactory;
  
  
  /** Constructs a new element of class_name 'name' and constructor
      arguments 'args'.  Returns reference to the constructed element */
  static ElementPtr
  mk(string name, TuplePtr args);


  /** Returns a string containing all elements known to me */
  static std::string
  elementList();


  /** An exception thrown when a factory fails to locate a named
      element. */
  struct ElementNotFound {
  public:
    ElementNotFound(std::string name);
    
    std::string elementName;
  };


  /** Registers an element factory given its user-visible name (e.g.,
      Identity, etc..) */
  static bool
  add(std::string elementName,
      ElementFactory factory);




private:
  /** No one should be creating a registry object */
  ElementRegistry();
  
  
  /** Implements the construct before use pattern */
  static void
  ensureInit();


  /** The factory functions for each registered factory method, indexed
      by the Element name given given at registration time. */
  typedef std::map<string,
                   ElementFactory,
                   std::less< string > > FactoryMap;
  
  
  /** The actual directory */
  static FactoryMap* _factories;


  /** A static initializer object to initialize static class objects */
  class Initializer {
  public:
    Initializer();
  };


  /** And the actual dummy initializer object.  Its constructor is the
      static initializer. */
  static Initializer*
  _initializer;


  /** Return the initializer ensuring it runs first */
  static Initializer*
  theInitializer();
};


/** The following macros are used in the private/public parts of element
    .h files, as well as the .C files to have elements be registered */

#define DECLARE_PRIVATE_ELEMENT_INITS           \
  class Initializer {                           \
  public:                                       \
    Initializer();                              \
  };                                            \
                                                \
  static Initializer*                           \
  theInitializer();                             \
                                                \


#define DECLARE_PUBLIC_ELEMENT_INITS                    \
  static ElementPtr                                     \
  mk(TuplePtr tp);                                      \
                                                        \
  static void                                           \
  ensureInit();                                         \
                                                        \
  

#define DEFINE_ELEMENT_INITS(_classname,_name)       \
  _classname::Initializer::Initializer()             \
  {                                                  \
    ElementRegistry::add(_name, &_classname::mk);    \
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
  ElementPtr                                         \
  _classname::mk(TuplePtr tp)                        \
  {                                                  \
    ElementPtr e(new _classname(tp));                \
    return e;                                        \
  }                                                  \
                                                     \


#define DEFINE_ELEMENT_INITS_NS(_classname,_name,_ns)   \
  DEFINE_ELEMENT_INITS(_classname,_name)



#endif /* __ELEMENTREGISTRY_H__ */
