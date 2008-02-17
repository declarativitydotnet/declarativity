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


#ifndef __VAL_GAUSSIAN_FACTOR_H__
#define __VAL_GAUSSIAN_FACTOR_H__

#include "value.h"
#include "oper.h"
#include "val_double.h"
#include "val_list.h"
#include "val_matrix.h"
#include "val_vector.h"
#include "val_factor.h"

#include <prl/variable.hpp>
#include <prl/gaussian_factors.hpp>
#include <prl/math/bindings/lapack.hpp>

class Val_Gaussian_Factor : public Val_Factor {
public:
  //! The matrix representation
  typedef prl::math::bindings::lapack::double_matrix matrix_type;

  //! The vector representation
  typedef prl::math::bindings::lapack::double_vector vector_type;

  //! The type of factors stored in this value
  typedef prl::canonical_gaussian<matrix_type, vector_type> canonical_gaussian;

  //! The moment Gaussian factor type (used for mean, covariance)
  typedef prl::moment_gaussian<matrix_type, vector_type> moment_gaussian;

  // Required type information
  const Value::TypeCode typeCode() const { return Value::GAUSSIAN_FACTOR; };
  const char *typeName() const { return "gaussian_factor"; };

  static const opr::Oper* oper_;

  // String representation. What exactly is toConfString()?
  std::string toString() const;

  // Serialization
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Gaussian_Factor() { }
  Val_Gaussian_Factor(const canonical_gaussian& factor) : factor(factor) { }
  ~Val_Gaussian_Factor() {};

  // Casting
  static const canonical_gaussian& cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  // Factory
  static ValuePtr mk();
  static ValuePtr mk(const canonical_gaussian& factor);
  static ValuePtr mk(ListPtr args, MatrixPtr lambda, VectorPtr eta);

  // Comparison
  int compareTo(ValuePtr v) const;

  // Accessors
  //! Returns the arguments of this factor
  domain_type arguments() const;

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
  ValuePtr multiply(ValuePtr factor) const;

  //! Collapses a factor to a subset of variables
  ValuePtr marginal(ListPtr retain) const;

  //! Computes a normalized version of a factor
  ValuePtr normalize() const;

 private:
  //! The factor stored in this value
  canonical_gaussian factor;
};

#endif /* __VAL_GAUSSIAN_FACTOR_H_*/
