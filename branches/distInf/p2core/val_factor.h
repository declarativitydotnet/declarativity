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

#include "value.h"
#include "oper.h"
#include "val_double.h"
#include "val_list.h"
#include "val_matrix.h"
#include "val_vector.h"

#include <prl/variable.hpp>
#include <prl/gaussian_factors.hpp>
#include <prl/math/bindings/lapack.hpp>

class Val_Factor : public Value {
public:
  //! The matrix representation
  typedef prl::math::bindings::lapack::double_matrix matrix_type;

  //! The vector representation
  typedef prl::math::bindings::lapack::double_vector vector_type;

  //! The type of factors stored in this value
  typedef prl::canonical_gaussian<matrix_type, vector_type> canonical_gaussian;

  //! The moment Gaussian factor type (used for mean, covariance)
  typedef prl::moment_gaussian<matrix_type, vector_type> moment_gaussian;

  //! The type of argument sets used in the factors
  typedef canonical_gaussian::domain_type domain_type;

  //! The type of variable handles (pointers)
  typedef prl::variable_h variable_h;

  //! A map from variable names to handles
  typedef prl::map<std::string, variable_h> named_var_map;

  // Required type information
  const Value::TypeCode typeCode() const { return Value::FACTOR; };
  const char *typeName() const { return "factor"; };

  static const opr::Oper* oper_;

  // String representation. What exactly is toConfString()?
  std::string toString() const;
  std::string toConfString() const;

  // Serialization
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Factor() { }
  Val_Factor(const canonical_gaussian& factor) : factor(factor) { }
  ~Val_Factor() {};

  // Casting
  static const canonical_gaussian& cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  // Factory
  static ValuePtr mk();
  static ValuePtr mk(const canonical_gaussian& factor);
  static ValuePtr mk(ListPtr args, MatrixPtr lambda, VectorPtr eta);

  //! Registers the local variable by name
  static variable_h registerVectorVariable(std::string name, std::size_t size);

  // Comparison
  int compareTo(ValuePtr v) const;

  // Accessors
  //! What exactly are we supposed to return here? Why is this important?
  unsigned int size() const;

  //! Returns the list of arguments of this factor
  ValuePtr arguments() const;

  /**
   * Returns the mean of the distribution represented by this factor
   * @return a pointer to a Val_Vector object that represents the mean
   * @note throws an assertion violation / exception if the
   *       factor is not normalizable.
   */
  ValuePtr mean() const;

  /**
   * Returns the covariance of the distirbution represented by this factor
   * @return a pointer to a Val_Matrix object that represents the covariance
   * @note throws an assertion violation / exception if the
   *       factor is not normalizable.
   */
  ValuePtr covariance() const;

  // Factor operations
  //! Multiplies two factors together
  static ValuePtr multiply(const canonical_gaussian& f1,
                           const canonical_gaussian& f2);

  //! Collapses a factor to a subset of variables
  static ValuePtr marginal(const canonical_gaussian& f1, ListPtr retain);

 private:
  //! A functor that transforms a ValuePtr to a double
  struct ValuePtr2double {
    double operator()(const ValuePtr p) { return Val_Double::cast(p); }
  };

  //! A functor that transforms a double to a pointer to Val_Double
  struct double2ValuePtr {
    ValuePtr operator()(const double& x) { return Val_Double::mk(x); }
  };

  //! Converts a list of variable names to a vector of variable handles
  static std::vector<variable_h> lookupVars(ListPtr list_ptr);

  //! The factor stored in this value
  canonical_gaussian factor;

  //! The set of all variables known to this host.
  static prl::universe u;

  //! The variable corresponding to each name
  static named_var_map named_var;
};

#endif /* __VAL_FACTOR_H_*/
