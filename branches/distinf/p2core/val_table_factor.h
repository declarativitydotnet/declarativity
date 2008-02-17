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


#ifndef __VAL_TABLE_FACTOR_H__
#define __VAL_TABLE_FACTOR_H__

#include "value.h"
#include "oper.h"
#include "val_double.h"
#include "val_list.h"
#include "val_matrix.h"
#include "val_factor.h"

#include <prl/variable.hpp>
#include <prl/table_factor.hpp>
#include <prl/math/bindings/lapack.hpp>

class Val_Table_Factor : public Val_Factor {
public:
  //! The type of factors stored in this value
  typedef prl::table_factor<double> factor_type;

  // Required type information
  const Value::TypeCode typeCode() const { return Value::TABLE_FACTOR; };
  const char *typeName() const { return "table_factor"; };

  static const opr::Oper* oper_;

  // String representation. What exactly is toConfString()?
  std::string toString() const;

  // Serialization
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Table_Factor() { }
  Val_Table_Factor(const factor_type& factor) : factor(factor) { }
  ~Val_Table_Factor() {};

  // Casting
  static const factor_type& cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  // Factory
  static ValuePtr mk();
  static ValuePtr mk(const factor_type& factor);
  static ValuePtr mk(ListPtr args, MatrixPtr values);

  // Comparison
  int compareTo(ValuePtr v) const;

  // Accessors
  //! Returns the arguments of this factor
  domain_type arguments() const;

  // Factor operations
  //! Multiplies two factors together
  ValuePtr multiply(ValuePtr factor) const;

  //! Collapses a factor to a subset of variables
  ValuePtr marginal(ListPtr retain) const;

  //! Computes a normalized version of a factor  
  ValuePtr normalize() const;
  
  //! Returns the values of this factor as a vector
  ValuePtr values() const;

 private:
  //! The factor stored in this value
  factor_type factor;
};

#endif /* __VAL_TABLE_FACTOR_H_*/
