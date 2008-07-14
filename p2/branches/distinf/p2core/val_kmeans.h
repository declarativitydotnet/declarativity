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
 
 
#ifndef __VAL_KMEANS_H__
#define __VAL_KMEANS_H__
 
#include "value.h"
#include "oper.h"

class Val_Kmeans : public Value {    

public:

  // Required fields for all concrete types.
  // The type name
  const Value::TypeCode typeCode() const { return Value::KMEANS; };
  const char *typeName() const { return "kmeans"; };
  // Print the matrix as a string.
  string toString() const ;
  virtual string toConfString() const;

  // Marshal/unmarshal a matrix.
  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Kmeans(int64_t i) {kmeans = i;}
  virtual ~Val_Kmeans() {};

  virtual unsigned int size() const { return sizeof(int64_t); }
  // Factory
  static ValuePtr mk(int64_t i) { ValuePtr p(new Val_Kmeans(i)); return p; };
  
  // strict comparison
  int compareTo(ValuePtr v) const;
  
  // Casting methods;
  static int64_t cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }
         
  static const opr::Oper* oper_;
   
private:
   int64_t kmeans;
};

#endif /* __VAL_KMEANS_H_*/

/* 
 * End of file
 */
