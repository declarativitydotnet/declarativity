/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776,
 * Berkeley, CA,  94707. Attention: P2 Group.
 *
 * DESCRIPTION: P2's concrete type system: the factor type.
 *
 */


#ifndef __VAL_FACTOR_H__
#define __VAL_FACTOR_H__

// #define PRL_PRINT_VARIABLE_ADDRESS

#include "value.h"
#include "oper.h"
#include "val_double.h"
#include "val_list.h"

#include <boost/thread/mutex.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <prl/variable.hpp>

class Val_Factor : public Value {
 public:
  //! The type of argument sets used in the factors
  typedef prl::domain_t domain_type;

  //! The type of variable handles (pointers)
  typedef prl::variable_h variable_h;

  //! A map from variable names to handles
  typedef prl::map<std::string, variable_h> named_var_map;

  // Constructors
  Val_Factor() { }
  ~Val_Factor() { };

  std::string toConfString() const;

  //! Registers the local variable by name
  static variable_h 
    registerVectorVariable(const std::string& name, std::size_t size);
  static variable_h
    registerFiniteVariable(const std::string& name, std::size_t size);

  // Accessors
  //! What exactly are we supposed to return here? Why is this important?
  unsigned int size() const;

  //! Returns the list of arguments of this factor
  virtual domain_type arguments() const = 0;
  ValuePtr args() const;

  //! Multiplies two factors together
  virtual ValuePtr multiply(ValuePtr factor) const = 0;

  //! Collapses this factor to a subset of variables
  virtual ValuePtr marginal(ListPtr retain) const = 0;

  //! Computes a normalized version of a factor
  virtual ValuePtr normalize() const = 0;

 protected:
  //! A functor that transforms a ValuePtr to a double
  struct ValuePtr2double {
    double operator()(const ValuePtr p) { return Val_Double::cast(p); }
  };

  //! A functor that transforms a double to a pointer to Val_Double
  struct double2ValuePtr {
    ValuePtr operator()(const double& x) { return Val_Double::mk(x); }
  };

  //! Converts a list of variable names to a vector of variable handles
  static std::vector<variable_h> 
    lookupVars(ListPtr list_ptr, bool ignore_missing = false);

  //! The set of all variables known to this host.
  static prl::universe u;

  //! The variable corresponding to each name
  static named_var_map named_var;

  static boost::mutex mutex;
};

#endif /* __VAL_FACTOR_H_*/
