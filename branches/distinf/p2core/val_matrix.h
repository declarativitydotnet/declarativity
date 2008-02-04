/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: the matrix type.  Based on
 * boost::numeric::ublas::matrix.
 * 
 */
 
 
#ifndef __VAL_MATRIX_H__
#define __VAL_MATRIX_H__
 
#include "value.h"
#include "oper.h"
#include <boost/numeric/ublas/matrix.hpp>

namespace ublas = boost::numeric::ublas;

typedef boost::shared_ptr< ublas::matrix< ValuePtr > > MatrixPtr;
typedef ublas::matrix< ValuePtr > ValPtrMatrix;

class Val_Matrix : public Value {    

public:

  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::MATRIX; };
  const char *typeName() const { return "matrix"; };
  // Print the matrix as a string.
  string toString() const ;
  virtual string toConfString() const;

  // Marshal/unmarshal a matrix.
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Matrix(MatrixPtr mp) : M(mp) {};
  Val_Matrix(uint64_t &size1, uint64_t &size2) { MatrixPtr p(new ValPtrMatrix(size1,size2)); M = p;}
  virtual ~Val_Matrix() {};

  // Factory
  static ValuePtr mk(MatrixPtr mp) { ValuePtr p(new Val_Matrix(mp)); return p; };
  static ValuePtr mk2(uint64_t &size1, uint64_t &size2) { ValuePtr p(new Val_Matrix(size1,size2)); return p; };
  
  // strict comparison
  int compareTo(ValuePtr v) const;
  
  ValuePtr transpose(); 
  // Get the size of the matrix as an unsigned int.
  virtual unsigned int size() const { return (M ? M->size1()*M->size2() : 0); }
  virtual unsigned int size1() const { return (M ? M->size1() : 0); }
  virtual unsigned int size2() const { return (M ? M->size2() : 0); }
  
  // Casting methods; only relevant to extract underlying matrix.
  static MatrixPtr cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  // manipulate matrix entries
  void insert(uint64_t &i1, uint64_t &i2, ValuePtr vp) { (*M)(i1,i2) = vp; };
  void erase(uint64_t &i1, uint64_t &i2) { (*M)(i1,i2).reset(); };

      
  static const opr::Oper* oper_;
   
private:
   MatrixPtr M;
};

#endif /* __VAL_MATRIX_H_*/

/* 
 * End of file
 */
