/*
 * @(#)$Id$
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

#include "math.h"
#include "value.h"
#include "oper.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/gregorian/gregorian.hpp"

const long PTIME_FRACTIONAL_FACTOR 
    = (long)exp10(boost::posix_time::time_duration::num_fractional_digits());
const long PTIME_SECS_FACTOR 
    = (long)exp10(9 - boost::posix_time::time_duration::num_fractional_digits());

class Val_Time : public Value {

public:  
  // Required fields for all concrete types.

  const Value::TypeCode typeCode() const { return Value::TIME; };

  // The type name
  const char *typeName() const { return "time"; };

  virtual string toConfString() const;
  virtual string toString() const {
    ostringstream sb;
    sb << "[" << to_simple_string(t) << "]";
    return sb.str();
  }
  virtual unsigned int size() const { return sizeof(boost::posix_time::ptime); }

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );

  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Time(string theTime) : 
    t(boost::posix_time::time_from_string(theTime)) {};
  Val_Time(boost::posix_time::ptime theTime) : t(theTime) {};
  Val_Time(struct timespec theTime);

  // Factory
  static ValuePtr mk(boost::posix_time::ptime theTime) 
    { return ValuePtr(new Val_Time(theTime)); };
  static ValuePtr mk(struct timespec ts) 
    { return ValuePtr(new Val_Time(ts)); };
  static ValuePtr mk(string ts) 
    { return ValuePtr(new Val_Time(ts)); };

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting
  static boost::posix_time::ptime cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }

  static const opr::Oper* oper_;
private:
  /** The time */
  boost::posix_time::ptime t;

  /** A double to be used with modf */
  static double _theDouble;
   
};

class Val_Time_Duration : public Value {

public:  
  // Required fields for all concrete types.

  const Value::TypeCode typeCode() const { return Value::TIME_DURATION; };

  // The type name
  const char *typeName() const { return "time_duration"; };

  virtual string toConfString() const;

  virtual string toString() const {
    ostringstream sb;
    sb << "[" << to_simple_string(td) << "]";
    return sb.str();
  }
  virtual unsigned int size() const { return sizeof(boost::posix_time::time_duration);};

  // Marshalling and unmarshallng
  void xdr_marshal_subtype( XDR *x );

  static ValuePtr xdr_unmarshal( XDR *x );

  // Constructors
  Val_Time_Duration(boost::posix_time::time_duration theDuration) : td(theDuration) {};

  // Factory
  static ValuePtr mk(boost::posix_time::time_duration theDuration) 
    { return ValuePtr(new Val_Time_Duration(theDuration)); };

  // Strict comparison
  int compareTo(ValuePtr) const;

  // Casting
  static boost::posix_time::time_duration cast(ValuePtr v);
  const ValuePtr toMe(ValuePtr other) const { return mk(cast(other)); }


  static const opr::Oper* oper_;
private:
  /** The duration */
  boost::posix_time::time_duration td;
  /** A double to be used with modf */
  static double _theDouble;
  
};

#endif /* __VAL_TIME_H_ */
