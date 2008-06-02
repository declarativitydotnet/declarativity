/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: the kmeans type. 
 * 
 */
 
 
#ifndef __VAL_GAUSSIAN_MIXTURE_H__
#define __VAL_GAUSSIAN_MIXTURE_H__
 
#include "value.h"
#include "val_factor.h"
#include "oper.h"
#include <iostream>
#include <string>

#include <prl/math/bindings/lapack.hpp>
#include <prl/factor/gaussian_factors.hpp>
#include <prl/factor/mixture.hpp>

using namespace std;
using namespace prl;


class Val_Gaussian_Mixture : public Val_Factor {

public:
  //! The matrix representation
  typedef math::bindings::lapack::double_matrix matrix_type;

  //! The vector representation
  typedef math::bindings::lapack::double_vector vector_type;

  //! The type of factors stored in this value
  typedef moment_gaussian<matrix_type, vector_type> factor_type;
  
  typedef mixture< factor_type > mixture_type;
  
  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::GAUSSIAN_MIXTURE; };
  const char *typeName() const { return "gaussian_mixture"; };

  // Print the mixture as a string.
  string toString() const ;

  // Marshal/unmarshal a matrix.
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Gaussian_Mixture() {};
  Val_Gaussian_Mixture(const mixture_type& mixture) : mixture(mixture) { }
  virtual ~Val_Gaussian_Mixture() {};

  virtual unsigned int size() const { return sizeof(int64_t); }
  
  // Factory
  static ValuePtr mk();
  static ValuePtr mk(const mixture_type& mixture);
  
  // strict comparison
  int compareTo(ValuePtr v) const;
  
  // Casting methods;
  static const mixture_type& cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }
         
  static const opr::Oper* oper_;

  prl::domain arguments() const;

  ValuePtr multiply(ValuePtr other) const;

  ValuePtr marginal(ListPtr retain) const;

  ValuePtr normalize() const;
  
private:
  mixture_type mixture;
};



#endif /* __VAL_GAUSSIAN_MIXTURE_H_*/

/* 
 * End of file
 */
