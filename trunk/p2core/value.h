/*
 * @(#)$Id$
 *
 * Copyright (c) 2004 Intel Corporation. All rights reserved.
 *
 * This file is distributed under the terms in the attached INTEL-LICENSE file.
 * If you do not find these files, copies can be found by writing to:
 * Intel Research Berkeley, 2150 Shattuck Avenue, Suite 1300,
 * Berkeley, CA, 94704.  Attention:  Intel License Inquiry.
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
 *    What goes in place of ## depends on the way in which you'd like to 
 *    define your operator function for your new type. There are a few ways we 
 *    can do this, which I've listed below. 
 *    1. The easiest way is to use OperImpl<Val_NAME>() template class, which
 *       defines the most basic operator functions. If you fit this category
 *       then this step is done. See type Val_Int32 for an example of this case.
 *    2. If your new type does not really fit the basic operator function templete
 *       you'll need to define your own subclass of Oper. There are a couple of 
 *       ways to do this.
 *       1. Lets say that your new type needs special attention for the math or bit 
 *          operator functions only and the basic comparison function will do just fine.
 *          Then your class can simply inherit from class OperCompare<Val_NAME> and
 *          all the basic comparison functions are given to you for free. You are then
 *          responsible for defining the missing math and bit operator functions in your
 *          OperNAME subclass. See Val_Str type for an example of this case.
 *       2. If for some reason your new type require special attention in the comparison
 *          operator functions then you'll need to inherit directly from class Oper. This
 *          will require you to define those operator functions that you wish to support.
 *          No known examples of this case since you should be overriding Value::compareTo 
 *          method!
 *       NOTE:
 *          Any operator function that is not defined in some subclass of Oper
 *          will cause an Oper::Exception to be thrown when that operator is applied 
 *          to your types and your type is the base type of the operands. 
 * 6. Register the operator function of our new type. To do this go to oper.C and follow 
 *    the instructions for filling in the operator table (oper_table_). You'll be filling
 *    the table with the Val_NAME::oper_ static member variable that we created and 
 *    initialized in the previous step... Depending on the base type you may be using
 *    other type's oper_ variable.
 * 7. Compile and done.
 */

#ifndef __VALUE_H__
#define __VALUE_H__

#include <boost/shared_ptr.hpp>
#include <vector>

#include <assert.h>
#include <async.h>
#include <arpc.h>
#include "inlines.h"

class Value;
typedef boost::shared_ptr<Value> ValuePtr;

class Value {

protected: 

  Value() {};
  virtual ~Value() {};

public:  

  // Type codes
  enum TypeCode { 
    NULLV=0,
    STR,
    INT32,
    UINT32,
    INT64,
    UINT64,
    DOUBLE,
    OPAQUE,
    TUPLE,
    TIME,
    ID,
    IP_ADDR,
    /* NEW TYPE DEFINITIONS GO HERE */
    TYPES
  };

  virtual unsigned int size() const = 0;
  
  // The type name
  virtual const TypeCode typeCode() const =0;
  virtual const char *typeName() const =0;

  // Conversions to strings: mandatory. 
  virtual str toString() const =0;
  str toTypeString() { return strbuf() << typeName() << ":" << toString();};

  /** Strict equality */
  REMOVABLE_INLINE bool equals( ValuePtr other ) const { return compareTo(other) == 0; }

  /** Am I less than, equal or greater than the other value?  -1 means
      less, 0 means equal, +1 means greater. */
  virtual int compareTo( ValuePtr other ) const = 0;

  // Thrown when an invalid type conversion is attempted. 
  struct TypeError { 
    TypeCode	realType;
    TypeCode	toType;
    TypeError(TypeCode t1, TypeCode t2) : realType(t1), toType(t2) {};
  };

  // Marshalling
  void xdr_marshal( XDR *x );
  static ValuePtr xdr_unmarshal( XDR *x );

protected:
  virtual void xdr_marshal_subtype( XDR *x )=0;
};

 
#endif /* __VALUE_H_ */
