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
#include <prl/factor/polymorphic_factor.hpp>

typedef prl::polymorphic_factor<double> polymorphic_factor;

class Val_Factor : public Value {
 public:
  /////////////////////////////////////////////////////////
  // Public types
  ////////////////////////////////////////////////////////

  //! The type of argument sets used in the factors
  typedef prl::domain domain;

  //! The type of variable handles (pointers)
  typedef prl::variable_h variable_h;

  //! A map from variable names to handles
  typedef prl::map<std::string, variable_h> named_var_map;

  // Required type information
  const Value::TypeCode typeCode() const { return Value::FACTOR; };
  const char *typeName() const { return "factor"; };

  static const opr::Oper* oper_;

  /////////////////////////////////////////////////////////
  // Public member functions
  ////////////////////////////////////////////////////////

  // Constructors
  Val_Factor() { }
  ~Val_Factor() { };

  static ValuePtr mk();
  static ValuePtr mk(const factor& f);
  static ValuePtr mk(const polymorphic_factor& f);
  static ValuePtr mk(ListPtr args, MatrixPtr values); // creates a table factor

  // Casts
  static polymorphic_factor<double> cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  // String representation. What exactly is toConfString()?
  std::string toString() const;
  std::string toConfString() const;

  // Serialization
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  //! Registers the local variable by name
  static variable_h 
    registerVectorVariable(const std::string& name, std::size_t size);

  static variable_h
    registerFiniteVariable(const std::string& name, std::size_t size);

  static variable_h
    registerVariable(const std::string& name, std::size_t size,
                     prl::variable::variable_type type);
  
  // Comparison
  int compareTo(ValuePtr v) const;

  // Accessors
  //! What exactly are we supposed to return here? Why is this important?
  unsigned int size() const;

  //! Returns the list of arguments of this factor
  domain arguments() const;

  //! Returns a list of argument names
  ValuePtr arg_names() const;

  //! Computes a marginal of a subset of variables
  ValuePtr marginal(ListPtr retain) const;
  
  //! Computes a maximum over a subset of variables
  ValuePtr maximum(ListPtr retain) const;

  //! Computes a minimum over a subset of variables
  ValuePtr minimum(ListPtr retain) const;

  //! Restricts the factor to an assignment of variables to values
  ValuePtr restrict(ListPtr variables, ListPtr values) const;
  
  //! Computes a normalized version of a factor
  ValuePtr normalize() const;

  //! Returns the values of a discrete factor as a vector
  ValuePtr values() const;

  /**
   * Returns the mean of the distribution represented by this factor
   * @return a pointer to a Val_Vector object that represents the mean
   * @note throws an assertion violation / exception if the
   *       factor is not convertible to a Gaussian or not normalizable.
   */
  ValuePtr mean() const;

  /**
   * Returns the covariance of the distirbution represented by this factor
   * @return a pointer to a Val_Matrix object that represents the covariance
   * @note throws an assertion violation / exception if the
   *       factor is not convertible to a Gaussian or not normalizable.
   */
  ValuePtr covariance() const;

  //! Converts a list of variable names to a vector of variable handles
  static std::vector<variable_h> 
    lookupVars(ListPtr list_ptr, bool ignore_missing = false);

 private:
  //! A functor that transforms a ValuePtr to a double
  struct ValuePtr2double {
    double operator()(const ValuePtr p) { return Val_Double::cast(p); }
  };

  //! A functor that transforms a double to a pointer to Val_Double
  struct double2ValuePtr {
    ValuePtr operator()(const double& x) { return Val_Double::mk(x); }
  };

  //! The underlying factor
  polymorphic_factor f;

  //! The set of all variables known to this host.
  static prl::universe u;

  //! The variable corresponding to each name
  static named_var_map named_var;

  static boost::mutex mutex;
};

// Free functions
//! Converts a list of variable names to a vector of variable handles
prl::var_vector varVector(ValuePtr vars);

//! Converts a list of variable names to a domain
prl::domain toDomain(ValuePtr vars);

//! Converts a list of variable names and values to an assignment
prl::assignment toAssignment(ValuePtr vars, ValuePtr vals);

//! Converts a textual representation of a vector to a double vector
double_vector parseVector(ValuePtr text);

//! Converts a textual representation of a matrix to a double matrix
double_matrix parseMatrix(ValuePtr text);

#endif /* __VAL_FACTOR_H_*/
