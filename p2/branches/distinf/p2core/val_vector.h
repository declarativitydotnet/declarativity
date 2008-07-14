/*
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: the vector type.  Based on
 * boost::numeric::ublas::vector.
 * 
 */
 
 
#ifndef __VAL_VECTOR_H__
#define __VAL_VECTOR_H__
 
#include "value.h"
#include "oper.h"
#include <boost/numeric/ublas/vector.hpp>

typedef boost::shared_ptr< boost::numeric::ublas::vector< ValuePtr > > VectorPtr;
typedef boost::numeric::ublas::vector< ValuePtr > ValPtrVector;
typedef boost::numeric::ublas::vector< double > doubleVector;

class Val_Vector : public Value {    

public:

  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::VECTOR; };
  const char *typeName() const { return "vector"; };
  // Print the vector as a string.
  string toString() const ;
  virtual string toConfString() const;

  // Marshal/unmarshal a vector.
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Vector(VectorPtr vp) : V(vp) {};
  Val_Vector(uint64_t &size) { VectorPtr p(new ValPtrVector(size)); V = p;}
  Val_Vector(const doubleVector& vec);
  Val_Vector(const std::vector<double>& vec);
  virtual ~Val_Vector() {};

  // Factory
  static ValuePtr mk(VectorPtr vp) { ValuePtr p(new Val_Vector(vp)); return p; };
  static ValuePtr mk2(uint64_t &size) { ValuePtr p(new Val_Vector(size)); return p; };
  static ValuePtr mk(const doubleVector& vec) { ValuePtr p(new Val_Vector(vec)); return p; }
  static ValuePtr mk(const std::vector<double>& vec) { ValuePtr p(new Val_Vector(vec)); return p; }
  
  // strict comparison
  int compareTo(ValuePtr v) const;
   
  // Get the size of the vector as an unsigned int.
  virtual unsigned int size() const { return (V ? V->size() : 0); }
  
  // Casting methods; only relevant to extract underlying vector.
  static VectorPtr cast(ValuePtr v);
  static doubleVector cast_double(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  // manipulate vector entries
  void insert(uint64_t &i, ValuePtr vp) { (*V)[i] = vp; };
  void erase(uint64_t &i) { (*V)[i].reset(); };
  // ValuePtr operator[] (uint64_t &i) { return (*V)[i]; }

  static const opr::Oper* oper_;
   
private:
   VectorPtr V;
};

#endif /* __VAL_VECTOR_H_*/

/* 
 * End of file
 */
