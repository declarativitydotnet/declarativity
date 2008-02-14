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
 * DESCRIPTION: P2's concrete type system.  The base type for values.
 * So you want to add a new P2 concrete type... Please follow these steps.
 * 1. Define new class Val_NAME in files val_NAME.h and val_NAME.C substituting 
 *    NAME for the name of your new type.
 * 2. The new class must inherit from class Value and override necessary virtual
 *    methods. 
 * 3. Define a new TypeCode right before the TYPES definition.
 * 4. In the class definition create the following public static member 
 *    variable definition: 
 *        const static Oper *oper_;
 * 5. We must now initialize the oper_ member variable in the Val_NAME.C file.
 *    Here is how the initialization should look:
 *        const Oper* Val_NAME::oper_ = New Oper##();
 *    What goes in place of ## depends on the way in which you'd like
 *    to define your operator function for your new type. There are a
 *    few ways we can do this, which I've listed below. 
 *    1. The easiest way is to use OperImpl<Val_NAME>() template
 *       class, which defines the most basic operator functions. If
 *       you fit this category then this step is done. See type
 *       Val_Int64 for an example of this case. 
 *    2. If your new type does not really fit the basic operator
 *       function templete you'll need to define your own subclass of
 *       Oper. There are a couple of ways to do this.
 *       1. Let's say that your new type needs special attention for
 *          the math or bit operator functions only and the basic
 *          comparison function will do just fine. Then your class can
 *          simply inherit from class OperCompare<Val_NAME> and all
 *          the basic comparison functions are given to you for
 *          free. You are then responsible for defining the missing
 *          math and bit operator functions in your OperNAME
 *          subclass. See Val_Str type for an example of this case. 
 *       2. If for some reason your new type require special attention
 *          in the comparison operator functions then you'll need to
 *          inherit directly from class Oper. This will require you to
 *          define those operator functions that you wish to
 *          support. No known examples of this case since you should
 *          be overriding Value::compareTo method!
 *       NOTE:
 *          Any operator function that is not defined in some subclass
 *          of Oper will cause an Oper::Exception to be thrown when
 *          that operator is applied to your types and your type is
 *          the base type of the operands.  
 * 6. Register the operator function of our new type. To do this go to
 *    oper.C and follow the instructions for filling in the operator
 *    table (oper_table_). You'll be filling the table with the
 *    Val_NAME::oper_ static member variable that we created and
 *    initialized in the previous step... Depending on the base type
 *    you may be using other type's oper_ variable.
 * 7. Compile and done.
 */

#ifndef __VALUE_H__
#define __VALUE_H__

#include <boost/shared_ptr.hpp>
#include <vector>
#include <deque>
#include <string>
#include <sstream>

#include <assert.h>
#include <stdexcept>
#include "inlines.h"
#include "config.h"

#include "reporting.h"

#include <set>

extern "C" {
#include <rpc/rpc.h>
#include <rpc/xdr.h>
}

// deal with xdr portability issues (originally found on OS X 10.4)
#ifdef HAVE_XDR_U_INT32_T
#define xdr_uint32_t xdr_u_int32_t
#define xdr_uint64_t xdr_u_int64_t
#endif

// deal with exp10 portability issues (missing from gcc4 on OS X 10.4)
#ifndef HAVE_EXP10
#define exp10(n) 	pow(10.0,(n))
#endif /* NO_EXP10 */


using std::string;
using std::ostringstream;

class Value;
typedef boost::shared_ptr<Value> ValuePtr;
typedef std::deque<ValuePtr> ValueList;

class Value {
protected: 

  Value();
  virtual ~Value();

public:  

  // Type codes
  enum TypeCode { 
    NULLV=0,
    STR,
    INT64,
    DOUBLE,
    OPAQUE,
    TUPLE,
    TIME,
    ID,
    /* NEW TYPE DEFINITIONS GO UNDER HERE */
    TIME_DURATION,
    SET,
    LIST,
    VECTOR,
    MATRIX,
    GAUSSIAN_FACTOR,
    TABLE_FACTOR,
    /* NEW TYPE DEFINITIONS GO ABOVE HERE */    
    TYPES
  };

  virtual unsigned int size() const = 0;
  
  // The type name
  virtual const TypeCode typeCode() const =0;
  virtual const char *typeName() const =0;

  // Conversion to my type.  Should run the local static in the
  // implementations
  virtual const ValuePtr toMe(ValuePtr other) const = 0;

  /** Method should return a string representation of the
   *  value's constructor and initialization arguments. */
  virtual std::string toConfString() const =0;

  // Conversions to strings: mandatory. 
  virtual std::string toString() const =0;
  string toTypeString() {
    ostringstream s;
    s << typeName() << ":" << toString();
    return s.str();
  };

  /** Strict equality */
  REMOVABLE_INLINE bool equals( ValuePtr other ) const { 
    try { 
      return compareTo(other) == 0; 
    } catch (Value::TypeError e) {
      return false;
    }
  }

  /** Am I less than, equal or greater than the other value?  -1 means
      less, 0 means equal, +1 means greater.  This is intended to
      provide a symmetric comparison operator that imposes a total order
      over objects from all subclasses of Value.  This is not intended
      as a method that implements the common less than operator; as such
      it is not doing the intuitive thing, for example, when comparing
      two numbers of different types.  Common comparison operators are
      implemented by the oper template functions, performing appropriate
      implicit casts as they go. */
  virtual int compareTo(ValuePtr other) const = 0;

  // Thrown when an invalid type conversion is attempted. 
  struct TypeError : public std::exception { 
    TypeCode realType;

    const char* realTypeName;

    TypeCode toType;

    const char* toTypeName;

    const char* _message;

    TypeError(TypeCode t1, const char* t1Name,
              TypeCode t2, const char* t2Name);

    const char* what() const throw();
  };

  // Marshalling
  void xdr_marshal( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );


  /** A comparator object for values */
  struct Comparator
  {
    bool
    operator()(const ValuePtr first,
               const ValuePtr second) const;
  };
  
  

protected:
  virtual void xdr_marshal_subtype( XDR *x ) = 0;

};

 
/** A set of values */
typedef std::set< ValuePtr, Value::Comparator > ValueSet;


#endif /* __VALUE_H_ */
