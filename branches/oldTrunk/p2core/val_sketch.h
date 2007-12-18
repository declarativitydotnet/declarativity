/*
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
 * Or
 * UC Berkeley EECS Computer Science Division, 387 Soda Hall #1776, 
 * Berkeley, CA,  94707. Attention: P2 Group.
 * 
 * DESCRIPTION: P2's concrete type system: time type.
 *
 */

#ifndef __VAL_TIME_H__
#define __VAL_TIME_H__

#include "value.h"

class Val_Sketch : public Value {

 public:
  const Value::TypeCode typeCode() const 
  { 
    return Value::SKETCH; 
  };

  const char *typeName() const 
  {
    return "sketch";
  };

  virtual string toConfString() const;
  virtual string toString() const;

  virtual unsigned int size() const;

  void xdr_marshal_subtype( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

  // Construct a sketch with a given id that will produce the given error
  // bound (on _frequently occurring_ items) with a given probability.
  Val_Sketch(double errorBound, double errorProbability);
  virtual ~Val_Sketch() {};

  // Factory
  static ValuePtr mk(double errorBound, double errorProbability) 
  { return ValuePtr(new Val_Sketch(errorBound, errorProbability)); };

  // Casting methods are only relevant to extract the underlying sketch.
  static ValuePtr cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }
  
  // Get the frequency of a frequently occurring item (this won't work with
  // infrequent occurrences due to the nature of the sketch).
  uint64_t getFrequency(const std::string &objectName);

 private:
  std::map<std::string, uint64_t>& map;
}
